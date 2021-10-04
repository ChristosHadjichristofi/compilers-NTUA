#ifndef __AST_HPP__
#define __AST_HPP__

#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <bits/stdc++.h>
#include "../types/types.hpp"
#include "AST.hpp"
#include "../library/library.hpp"
#include "../error/error.hpp"

extern int comment_depth;

inline std::ostream & operator<< (std::ostream &out, const AST &ast) {
    ast.printOn(out);
    return out;
}

class Constant : public AST {};

class Expr : public AST {
public:

    virtual SymbolEntry *sem_getExprObj() { return nullptr; }

    virtual std::string getName() { return ""; }

    CustomType *getType() { return type; }

    void setType(CustomType *t) { this->type = t; }

    virtual std::pair<CustomType *, int> getRefFinalType(CustomType *ct) const {

        int levels = 1;
        CustomType *obj = ct;

        while (obj->ofType != nullptr && obj->typeValue == TYPE_REF) {
            levels++;
            obj = obj->ofType;
        }
        if (levels == 1) return std::make_pair(ct, levels);
        return std::make_pair(obj, levels);

    }

    virtual std::pair<CustomType *, int> getFnFinalType(CustomType *ct) const {

        int levels = 1;
        CustomType *obj = ct;

        while (obj->outputType != nullptr && obj->typeValue == TYPE_FUNC) {
            levels++;
            obj = obj->outputType;
        }
        if (levels == 1) return std::make_pair(ct, levels);
        return std::make_pair(obj, levels);

    }

    virtual llvm::Value* compile() const override {
        return 0;
    }

protected:
    CustomType *type = nullptr;
};

class Pattern : public Expr {
public:
    void setMatchExprV(llvm::Value *v) { matchExprV = v; }

    void setNextClauseBlock(llvm::BasicBlock *bb) { nextClauseBlock = bb; }

    llvm::Value *matchExprV;
    llvm::BasicBlock *nextClauseBlock;
};

class Block : public AST {
public:
    Block(): block() {}

    void appendBlock(Block *b) {
        block.push_back(b);
    }

    virtual void printOn(std::ostream &out) const override {

        out << "Block(";
        bool first = true;
        for (auto i = block.rbegin(); i != block.rend(); ++i) {
            if (!first) out << ", ";
            first = false;
            (*i)->printOn(out);
        }
        out<< ")";
    }

    virtual void sem() override {
        st.openScope();
        Library *l = new Library();
        l->init();
        st.openScope();
        for (auto i = block.rbegin(); i != block.rend(); ++i) (*i)->sem();
        st.closeScope();
        st.closeScope();
    }

    virtual llvm::Value* compile() const override {

        /* create string struct type */
        std::vector<llvm::Type *> members;
        /* ptr to array */
        members.push_back(llvm::PointerType::getUnqual(i8));
        /* dimensions number of array */
        members.push_back(i32);

        /* string is defined as an array of one dim */
        members.push_back(i32);

        /* create the struct */
        std::string arrName = "Array_String_1";
        llvm::StructType *arrayStruct = llvm::StructType::create(TheContext, arrName);
        arrayStruct->setBody(members);

        /* create unit struct (type opaque -> no body) */
        std::string unitName = "unit";
        llvm::StructType *unitType = llvm::StructType::create(TheContext, unitName);
        std::vector<llvm::Type *> emptyBody;
        // emptyBody.push_back(i1);
        unitType->setBody(emptyBody);

        currPseudoScope = currPseudoScope->getNext();
        currPseudoScope = currPseudoScope->getNext();
        for (auto i = block.rbegin(); i != block.rend(); ++i) (*i)->compile();
        currPseudoScope = currPseudoScope->getPrev();
        currPseudoScope = currPseudoScope->getPrev();
        return nullptr;
    }

protected:
    std::vector<Block*> block;
};

class ExprGen : public Expr {
public:
    ExprGen(Expr *e, ExprGen *eg): expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {

        if (exprGen == nullptr) {
            out << "ExprGen("; expr->printOn(out); out <<")";
        }
        else {
            out << "ExprGen("; expr->printOn(out); out << ", "; exprGen->printOn(out); out << ")";
        }

    }

    Expr *getExpr() { return expr; }

    ExprGen *getNext() { return exprGen; }

    virtual void sem() override {
        expr->sem();
        this->type = expr->getType();
        if (exprGen != nullptr) exprGen->sem();
    }

    virtual llvm::Value* compile() const override {
        return expr->compile();
    }

private:
    Expr *expr;
    ExprGen *exprGen;
};

class Id : public Expr {
public:
    Id(std::string n): name(n) {}
    Id(std::string n, Expr *e, ExprGen *eg): name(n), expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {
        if (exprGen == nullptr) {
            out << "Id(" << name;
            if (expr != nullptr) { out << ", "; expr->printOn(out); }
            out << ")";
        }
        else {
            out << "Id(" << name <<", "; expr->printOn(out); out <<", "; exprGen->printOn(out); out <<")";
        }
    }

    std::string getName() override { return name; }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(name); }

    virtual void sem() override {

        /* lookup for variable, if exists */
        if (expr == nullptr && exprGen == nullptr) {
            SymbolEntry *tempEntry = st.lookup(name);
            if (tempEntry != nullptr) this->type = tempEntry->type;
            else {
                /* Print Error First Occurence */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;

                Error *err = new FirstOccurence(name);
                err->printError();

                tempEntry = new SymbolEntry(name, new Unknown());
                /* Now Unknown is recognized as None */
                tempEntry->type->size = 0;
                this->type = tempEntry->type;
                tempEntry->entryType = ENTRY_TEMP;
                st.insert(name, tempEntry);
            }
        }
        /* lookup for function */
        else {
            SymbolEntry *tempEntry = st.lookup(name);

            if (tempEntry != nullptr) {

                /* check if calling a non function */
                if (expr != nullptr && tempEntry->params.empty() && tempEntry->type->typeValue != TYPE_UNKNOWN) {
                    CustomType *newFunc = new Function(new Unknown());
                    ExprGen *tempExprGen = exprGen;
                    expr->sem();
                    newFunc->params.push_back(expr->getType());

                    if (exprGen != nullptr) exprGen->sem();

                    while (tempExprGen != nullptr) {
                        if (tempExprGen->getNext() == nullptr) newFunc->outputType = tempExprGen->getType();
                        else newFunc->params.push_back(tempExprGen->getType());
                        tempExprGen = tempExprGen->getNext();
                    }

                    /* Print Error */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(tempEntry->type, newFunc);
                    err->printError();

                    this->type = new Unknown();
                    return;
                }

                if (tempEntry->type->typeValue == TYPE_UNKNOWN) {
                    std::string tempName;
                    int counter = 0;
                    SymbolEntry *se;
                    CustomType *ct = new Unknown();
                    tempEntry->type->~CustomType();
                    tempEntry->type = new (tempEntry->type) Function(new Unknown());

                    if (expr != nullptr) {
                        tempName = tempEntry->id + "_param_" + std::to_string(counter++);
                        dynamic_cast<Function *>(tempEntry->type)->params.push_back(ct);
                        se = new SymbolEntry(tempName, ct);
                        se->entryType = ENTRY_TEMP;
                        tempEntry->params.push_back(se);
                    }

                    if (exprGen != nullptr) {
                        ExprGen *tempExprGen = exprGen;
                        while (tempExprGen != nullptr) {
                            ct = new Unknown();
                            tempName = tempEntry->id + "_param_" + std::to_string(counter++);
                            dynamic_cast<Function *>(tempEntry->type)->params.push_back(ct);
                            se = new SymbolEntry(tempName, ct);
                            se->entryType = ENTRY_TEMP;
                            tempEntry->params.push_back(se);
                            tempExprGen = tempExprGen->getNext();
                        }
                    }
                }

                if (tempEntry->entryType == ENTRY_TEMP) {
                    Error *err;
                    CustomType *prevTempEntryType = new Unknown();
                    prevTempEntryType->size = 0;
                    this->type = prevTempEntryType;

                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    err = new TypeMismatch(prevTempEntryType, st.nonTempLookup(name)->type);
                    err->printError();
                    return;
                }

                this->type = dynamic_cast<Function*>(tempEntry->type)->outputType;
                /* lookup for first param of a function */
                if (expr != nullptr) expr->sem();

                /* type inference */
                if (tempEntry->params.front()->type->typeValue == TYPE_ARRAY
                 && tempEntry->params.front()->type->ofType->typeValue == TYPE_UNKNOWN
                 && expr->getType()->typeValue == TYPE_ARRAY
                 && expr->getType()->ofType->typeValue != TYPE_UNKNOWN) tempEntry->params.front()->type->ofType = expr->getType()->ofType;

                if (tempEntry->params.front()->type->typeValue == TYPE_ARRAY
                 && tempEntry->params.front()->type->ofType->typeValue != TYPE_UNKNOWN
                 && expr->getType()->typeValue == TYPE_ARRAY
                 && expr->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                    // expr->setType(tempEntry->params.front()->type);
                    SymbolEntry *se = expr->sem_getExprObj();
                    se->type->ofType = expr->getType()->ofType;
                }
                if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->getType()->typeValue != TYPE_UNKNOWN) {
                    // Destroy the object but leave the space allocated.
                    CustomType *tempCT = tempEntry->params.front()->type;
                    std::string tempName;
                    if (expr->getType()->typeValue == TYPE_CUSTOM) {
                        SymbolEntry *exprObj = expr->sem_getExprObj();
                        tempName = exprObj->type->name;
                    }
                    tempCT->~CustomType();
                    // Create a new object in the same space.
                    if (expr->getType()->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                    else if (expr->getType()->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                    else if (expr->getType()->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                    else if (expr->getType()->typeValue == TYPE_ARRAY && expr->getType()->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                    else if (expr->getType()->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                    else if (expr->getType()->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                    else if (expr->getType()->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                    else if (expr->getType()->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(expr->getType()->ofType);
                    else if (expr->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = tempName; }

                    dynamic_cast<Function *>(tempEntry->type)->params.front() = tempEntry->params.front()->type;
                }

                /* edge case */
                if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->sem_getExprObj() != nullptr && expr->sem_getExprObj()->type->typeValue == TYPE_FUNC) {
                    // tempEntry->params.front()->type = expr->sem_getExprObj()->type->outputType;
                    expr->sem_getExprObj()->sameMemAsOutput = true;
                    expr->sem_getExprObj()->type->outputType = tempEntry->params.front()->type;
                    dynamic_cast<Function *>(tempEntry->type)->params.front() = tempEntry->params.front()->type;
                }

                if (tempEntry->params.front()->type->typeValue != TYPE_UNKNOWN
                 && expr->getType()->typeValue == TYPE_UNKNOWN
                 && expr->sem_getExprObj() != nullptr
                 && (expr->sem_getExprObj()->type->typeValue == TYPE_UNKNOWN || (expr->sem_getExprObj()->type->typeValue == TYPE_ARRAY && expr->sem_getExprObj()->type->ofType->typeValue == TYPE_UNKNOWN) || getRefFinalType(expr->sem_getExprObj()->type).first->typeValue == TYPE_UNKNOWN)) {

                    expr->setType(tempEntry->params.front()->type);

                    SymbolEntry *se = expr->sem_getExprObj();

                    if (se->type->typeValue == TYPE_ARRAY) se->type->ofType = expr->getType();
                    else if (se->type->typeValue == TYPE_REF) {
                        CustomType * yt = se->type;
                        std::string tempName;

                        if (expr->getType()->typeValue == TYPE_CUSTOM) {
                            SymbolEntry *exprObj = expr->sem_getExprObj();
                            tempName = exprObj->type->name;
                        }

                        while (yt->typeValue == TYPE_REF) yt = yt->ofType;

                        yt->~CustomType();

                        if (expr->getType()->typeValue == TYPE_INT) yt = new (yt) Integer();
                        else if (expr->getType()->typeValue == TYPE_FLOAT) yt = new (yt) Float();
                        else if (expr->getType()->typeValue == TYPE_CHAR) yt = new (yt) Character();
                        else if (expr->getType()->typeValue == TYPE_ARRAY && expr->getType()->ofType->typeValue == TYPE_CHAR) yt = new (yt) Array(new Character(), 1);
                        else if (expr->getType()->typeValue == TYPE_BOOL) yt = new (yt) Boolean();
                        else if (expr->getType()->typeValue == TYPE_UNIT) yt = new (yt) Unit();
                        else if (expr->getType()->typeValue == TYPE_UNKNOWN) yt = new (yt) Unknown();
                        else if (expr->getType()->typeValue == TYPE_CUSTOM) { yt = new (yt) CustomType(); yt->name = tempName; }
                    }
                    else se->type = expr->getType();
                }

                if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->getType()->typeValue == TYPE_UNKNOWN) {
                    SymbolEntry *se = expr->sem_getExprObj();
                    /* if expr is a function (as a param) and its return type unknown (common),
                       get expr->SymbolEntry and set the first param of tempEntry to its type */
                    if (se->type->typeValue == TYPE_FUNC) {
                        /* edge case for same function being given as a param to itself
                           aka it is in the same memory */
                        if (!se->params.empty() && tempEntry->params.front() == se->params.front()) {}
                        else {
                            tempEntry->params.front()->type = se->type;
                            tempEntry->type->params.front() = se->type;
                        }
                    }
                    else {
                        expr->setType(tempEntry->params.front()->type);
                        if (se->type->typeValue == TYPE_REF || se->type->typeValue == TYPE_ARRAY) se->type->ofType = expr->getType();
                        else se->type = expr->getType();
                    }
                }

                /* Check first param of function with given param */
                if (expr->getType()->typeValue != tempEntry->params.front()->type->typeValue) {
                    /* edge case for ref(unknown) to array(unknown) -> type correction */
                    if (expr->getType()->typeValue == TYPE_ARRAY && expr->getType()->ofType->typeValue == TYPE_UNKNOWN
                    && tempEntry->params.front()->type->typeValue == TYPE_REF && tempEntry->params.front()->type->ofType->typeValue == TYPE_UNKNOWN) {
                        tempEntry->params.front()->type = expr->getType();
                    }
                    // /* edge case for CustomId - CustomType type check */
                    else if (expr->getType()->typeValue == TYPE_CUSTOM && tempEntry->params.front()->type->typeValue == TYPE_CUSTOM
                          && expr->sem_getExprObj()->params.front()->id == tempEntry->params.front()->type->name) {
                        /* all good */
                    }
                    /* edge case for function */
                    else if (expr->getType()->typeValue == TYPE_FUNC || tempEntry->params.front()->type->typeValue == TYPE_FUNC) {
                        CustomType *tempFunc1 = expr->sem_getExprObj()->type;
                        CustomType *tempFunc2 = tempEntry->params.front()->type;
                        while (tempFunc1->typeValue == TYPE_FUNC) tempFunc1 = tempFunc1->outputType;
                        while (tempFunc2->typeValue == TYPE_FUNC) tempFunc2 = tempFunc2->outputType;
                        if (tempFunc1->typeValue != tempFunc2->typeValue) {
                            /* Print Error - type mismatch */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempFunc1, tempFunc2);
                            err->printError();
                        }
                    }
                    else{
                        /* Print Error - type mismatch */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(expr->getType(), tempEntry->params.front()->type);
                        err->printError();
                    }
                }
                else {
                    if (expr->getType()->typeValue == TYPE_ARRAY
                    && tempEntry->params.front()->type->typeValue == TYPE_ARRAY
                    && expr->getType()->size != tempEntry->params.front()->type->size) {
                        /* Print Error - type mismatch */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new ArrayDimensions(dynamic_cast<Array *>(expr->getType()), dynamic_cast<Array *>(tempEntry->params.front()->type));
                        err->printError();
                    }
                }

                /* if a expr SE is a function, make sure all its params get type inference/check
                   with the params of the function being called */
                if (expr->sem_getExprObj() != nullptr && expr->sem_getExprObj()->type->typeValue == TYPE_FUNC
                 && tempEntry->params.front() != nullptr && !tempEntry->params.front()->id.empty()) {
                    long unsigned int counter = 0;
                    SymbolEntry *exprEntry = expr->sem_getExprObj();
                    SymbolEntry *firstParamEntry = tempEntry->params.front();

                    while (!firstParamEntry->params.empty() && counter < firstParamEntry->params.size()) {
                        /* type inference */
                        if ((firstParamEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN && dynamic_cast<Function *>(firstParamEntry->type)->params.at(counter)->typeValue == TYPE_UNKNOWN)
                        || firstParamEntry->params.at(counter)->sameMemAsOutput
                        ) {

                            CustomType *tempCT = firstParamEntry->params.at(counter)->type;
                            tempCT->~CustomType();

                            // Create a new object in the same space.
                            if (exprEntry->params.at(counter)->type->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_ARRAY && exprEntry->params.at(counter)->type->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(exprEntry->params.at(counter)->type->ofType);
                            else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FUNC) {
                                tempCT = new (tempCT) Function(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->outputType);

                                long unsigned int index = 0;

                                // iterating through dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params and
                                // adding them to type function params
                                while (index < dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.size())
                                    dynamic_cast<Function *>(tempCT)->params.push_back(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.at(index++));

                            }
                        }
                        /* if param is a function it must change its (final) outputType */
                        if (firstParamEntry->params.at(counter)->type->typeValue == TYPE_FUNC) {
                            CustomType *tempFunc = firstParamEntry->params.at(counter)->type;
                            while (tempFunc->typeValue == TYPE_FUNC) tempFunc = tempFunc->outputType;
                            CustomType *exprCT = exprEntry->params.at(counter)->type;
                            tempFunc->~CustomType();

                            // Create a new object in the same space.
                            if (exprCT->typeValue == TYPE_INT) tempFunc = new (tempFunc) Integer();
                            else if (exprCT->typeValue == TYPE_FLOAT) tempFunc = new (tempFunc) Float();
                            else if (exprCT->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Character();
                            else if (exprCT->typeValue == TYPE_ARRAY && exprCT->ofType->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Array(new Character(), 1);
                            else if (exprCT->typeValue == TYPE_BOOL) tempFunc = new (tempFunc) Boolean();
                            else if (exprCT->typeValue == TYPE_UNIT) tempFunc = new (tempFunc) Unit();
                            else if (exprCT->typeValue == TYPE_UNKNOWN) tempFunc = new (tempFunc) Unknown();
                        }

                        /* type check */
                        CustomType *funcFinalType = firstParamEntry->params.at(counter)->type;

                        while (funcFinalType->typeValue == TYPE_FUNC) funcFinalType = funcFinalType->outputType;

                        if (funcFinalType->typeValue != exprEntry->params.at(counter)->type->typeValue) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(firstParamEntry->params.at(counter)->type, exprEntry->params.at(counter)->type);
                            err->printError();
                        }
                        counter++;
                    }
                    if (firstParamEntry->type->typeValue == TYPE_FUNC) {
                        // dynamic_cast<Function *>(firstParamEntry->type)->outputType = dynamic_cast<Function *>(exprEntry->type)->outputType;

                        CustomType *tempFunc = firstParamEntry->type->outputType;
                        CustomType *exprCT = exprEntry->type->outputType;

                        if (tempEntry->type->outputType->typeValue != TYPE_UNKNOWN) {
                            if (tempFunc->typeValue != exprCT->typeValue) {
                                /* Print Error - type mismatch */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempEntry->type->outputType, expr->sem_getExprObj()->type->outputType);
                            err->printError();
                            }
                        }
                        else {
                            tempFunc->~CustomType();

                            // Create a new object in the same space.
                            if (exprCT->typeValue == TYPE_INT) tempFunc = new (tempFunc) Integer();
                            else if (exprCT->typeValue == TYPE_FLOAT) tempFunc = new (tempFunc) Float();
                            else if (exprCT->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Character();
                            else if (exprCT->typeValue == TYPE_ARRAY && exprCT->ofType->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Array(new Character(), 1);
                            else if (exprCT->typeValue == TYPE_BOOL) tempFunc = new (tempFunc) Boolean();
                            else if (exprCT->typeValue == TYPE_UNIT) tempFunc = new (tempFunc) Unit();
                            else if (exprCT->typeValue == TYPE_UNKNOWN) tempFunc = new (tempFunc) Unknown();

                        }

                    }
                }

                if (tempEntry->params.front()->type->typeValue == TYPE_REF || expr->getType()->typeValue == TYPE_REF) {
                    std::pair <CustomType *, int> pairExpr1, pairExpr2;
                    pairExpr1 = getRefFinalType(tempEntry->params.front()->type);
                    pairExpr2 = getRefFinalType(expr->getType());
                    /* same but for Reference(Unknown()) */
                    if (pairExpr1.first->typeValue == TYPE_UNKNOWN && pairExpr2.first->typeValue == TYPE_UNKNOWN) {
                        /* Warning polymorphic value */
                        int levelsDiff = (pairExpr1.second > pairExpr2.second) ? pairExpr1.second - pairExpr2.second : pairExpr2.second - pairExpr1.second;
                        CustomType *t1 = pairExpr1.first;
                        CustomType *t2 = pairExpr2.first;
                        if (pairExpr1.second > pairExpr2.second) {
                            while (levelsDiff > 0) {
                                t2 = new Reference(new Unknown());
                                t2 = t2->ofType;
                                levelsDiff--;
                            }
                        }
                        if (pairExpr1.second < pairExpr2.second) {
                            while (levelsDiff > 0) {
                                t1 = new Reference(new Unknown());
                                t1 = t1->ofType;
                                levelsDiff--;
                            }
                        }
                    }
                    /* Check if either given param is of unknown type or function param is of unknown type - Type Inference */
                    else if (pairExpr1.first->typeValue == TYPE_UNKNOWN) {
                        if (pairExpr1.second > pairExpr2.second) {
                            /* Print Error - type mismatch */

                            /* keep the depth of expr2 in order to know how much to traverse
                               into expr1 types and print the error correctly */
                            int pairExpr2Depth = pairExpr2.second;
                            CustomType *traverseType = tempEntry->params.front()->type;
                            while (pairExpr2Depth != 1) { traverseType = traverseType->ofType; pairExpr2Depth--; }

                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(traverseType, pairExpr2.first);
                            err->printError();
                        }
                        else tempEntry->params.front()->type = expr->getType();
                    }
                    else if (pairExpr2.first->typeValue == TYPE_UNKNOWN) {
                        if (pairExpr1.second < pairExpr2.second) {
                            /* Print Error - type mismatch */

                            /* keep the depth of expr1 in order to know how much to traverse
                               into expr2 types and print the error correctly */
                            int pairExpr1Depth = pairExpr1.second;
                            CustomType *traverseType = expr->getType();
                            while (pairExpr1Depth != 1) { traverseType = traverseType->ofType; pairExpr1Depth--; }

                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(traverseType, pairExpr1.first);
                            err->printError();
                        }
                        else {
                            expr->setType(tempEntry->params.front()->type);
                            SymbolEntry *se = dynamic_cast<Id*>(expr)->sem_getExprObj();
                            se->type = expr->getType();
                        }
                    }
                    /* Check ith param given that has the same type as the ith param of the function */
                    else if (pairExpr1.first->typeValue != pairExpr2.first->typeValue || pairExpr1.second != pairExpr2.second ) {
                        /* Print Error - type mismatch */

                        int pairExprDepth;
                        CustomType *traverseType;

                        /* keep the depth of the expr that has the smallest depth
                            in order to know how much to traverse
                            into the other expr types and print the error correctly */
                        if (pairExpr1.second < pairExpr2.second) {
                            pairExprDepth = pairExpr1.second;
                            traverseType = expr->getType();
                        }
                        else {
                            pairExprDepth = pairExpr2.second;
                            traverseType = tempEntry->params.front()->type;
                        }

                        while (pairExprDepth != 1) { traverseType = traverseType->ofType; pairExprDepth--; }

                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;

                        Error *err;
                        if (pairExpr1.second < pairExpr2.second) err = new TypeMismatch(pairExpr1.first, traverseType);
                        else err = new TypeMismatch(traverseType, pairExpr2.first);
                        err->printError();
                    }
                }

                /* if there is only expr (no other exprs) and it's a function,
                   make this tempEntry's outputType equal to the outputType of expr */
                if (exprGen == nullptr && expr->sem_getExprObj() != nullptr && expr->sem_getExprObj()->type->typeValue == TYPE_FUNC && tempEntry->type->outputType->typeValue == TYPE_UNKNOWN) {
                    tempEntry->type->outputType = expr->sem_getExprObj()->type->outputType;
                }


                /* lookup for the rest params of a function, if they exist */
                if (exprGen != nullptr) {
                    exprGen->sem();
                    /* Check for the rest params of function with the rest given params */
                    ExprGen *tempExprGen = exprGen;
                    long unsigned int i = 1;
                    for (; (i < tempEntry->params.size() && tempExprGen != nullptr); i++, tempExprGen = tempExprGen->getNext()) {
                        /* Check if both function param and given param have unknown type */
                        if (tempEntry->params.at(i)->type->typeValue == TYPE_UNKNOWN && tempExprGen->getType()->typeValue == TYPE_UNKNOWN) {
                            tempExprGen->getExpr()->setType(tempEntry->params.at(i)->type);
                            SymbolEntry *exprGenEntry = tempExprGen->getExpr()->sem_getExprObj();
                            if (exprGenEntry->type->typeValue == TYPE_REF || exprGenEntry->type->typeValue == TYPE_ARRAY) exprGenEntry->type->ofType = tempExprGen->getExpr()->getType();
                            else exprGenEntry->type = tempExprGen->getExpr()->getType();
                        }
                        /* Check if either given param is of unknown type or function param is of unknown type - Type Inference */
                        else if (tempEntry->params.at(i)->type->typeValue == TYPE_UNKNOWN) {
                            // Destroy the object but leave the space allocated.
                            CustomType *tempCT = tempEntry->params.at(i)->type;
                            std::string ctName;
                            if (!tempCT->name.empty()) ctName = tempCT->name;
                            tempCT->~CustomType();

                            // Create a new object in the same space.
                            if (tempExprGen->getType()->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                            else if (tempExprGen->getType()->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                            else if (tempExprGen->getType()->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                            else if (tempExprGen->getType()->typeValue == TYPE_ARRAY && tempExprGen->getType()->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                            else if (tempExprGen->getType()->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                            else if (tempExprGen->getType()->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                            else if (tempExprGen->getType()->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                            else if (tempExprGen->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }

                            dynamic_cast<Function *>(tempEntry->type)->params.at(i) = tempEntry->params.at(i)->type;
                        }
                        else if (tempExprGen->getType()->typeValue == TYPE_UNKNOWN) {
                            tempExprGen->setType(tempEntry->params.at(i)->type);
                            SymbolEntry *se = tempExprGen->getExpr()->sem_getExprObj();

                            // Destroy the object but leave the space allocated.
                            CustomType *tempCT = se->type;
                            std::string ctName;
                            if (!tempEntry->params.at(i)->type->name.empty()) ctName = tempEntry->params.at(i)->type->name;
                            tempCT->~CustomType();

                            // Create a new object in the same space.
                            if (tempEntry->params.at(i)->type->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY && tempEntry->params.at(i)->type->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
                        }
                        /* Check ith param given that has the same type as the ith param of the function */
                        else {
                            
                        //     if (tempExprGen->getExpr()->sem_getExprObj() == nullptr) std::cout <<tempExprGen->getExpr()->getName() <<" is broken";
                        // tempExprGen->getExpr()->sem_getExprObj()->type->printOn(std::cout); std::cout.flush();
                            std::string nameOfType = "";
                            if (tempExprGen->getExpr()->sem_getExprObj() != nullptr) {
                                nameOfType = (tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_ID) ? tempExprGen->getExpr()->sem_getExprObj()->params.front()->type->name : tempExprGen->getExpr()->sem_getExprObj()->type->name;
                                if (tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_FUNC && tempExprGen->getExpr()->sem_getExprObj()->type->outputType->typeValue == TYPE_CUSTOM) nameOfType = tempExprGen->getExpr()->sem_getExprObj()->type->outputType->name;
                            }
                            
                            if (!nameOfType.compare("")) nameOfType = tempExprGen->getType()->name;
                            
                            if (tempExprGen->getType()->typeValue == TYPE_CUSTOM
                            && tempEntry->params.at(i)->type->typeValue == TYPE_CUSTOM
                            && tempEntry->params.at(i)->type->name != nameOfType) {
                                /* Print Error - type mismatch */
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(tempEntry->params.at(i)->type, tempExprGen->getType());
                                err->printError();
                            }
                            else if (tempExprGen->getType()->typeValue != TYPE_CUSTOM && tempEntry->params.at(i)->type->typeValue != TYPE_CUSTOM) {
                                if (tempEntry->params.at(i)->type->typeValue != tempExprGen->getType()->typeValue) {
                                    /* egde case for ref(unknown) to array(unknown) -> type correction */
                                    if (tempExprGen->getType()->typeValue == TYPE_ARRAY && tempExprGen->getType()->ofType->typeValue == TYPE_UNKNOWN
                                    && tempEntry->params.at(i)->type->typeValue == TYPE_REF && tempEntry->params.at(i)->type->ofType->typeValue == TYPE_UNKNOWN) {
                                        tempEntry->params.at(i)->type = expr->getType();
                                    }
                                    else{
                                        /* Print Error - type mismatch */
                                        semError = true;
                                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                        Error *err = new TypeMismatch(tempEntry->params.at(i)->type, tempExprGen->getType());
                                        err->printError();
                                    }
                                }
                                else {
                                    if (tempExprGen->getType()->typeValue == TYPE_ARRAY
                                    && tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY
                                    && tempExprGen->getType()->size != tempEntry->params.at(i)->type->size) {
                                        /* Print Error - type mismatch */
                                        semError = true;
                                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                        Error *err = new ArrayDimensions(dynamic_cast<Array *>(tempExprGen->getType()), dynamic_cast<Array *>(tempEntry->params.at(i)->type));
                                        err->printError();
                                    }
                                }
                            }
                        }

                        if (tempExprGen->getExpr()->getType()->typeValue == TYPE_FUNC) {

                            long unsigned int counter = 0;
                            SymbolEntry *exprEntry = tempExprGen->getExpr()->sem_getExprObj();
                            SymbolEntry *paramEntry = tempEntry->params.at(i);

                            while (counter < paramEntry->params.size()) {

                                /* type inference */
                                if (paramEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN
                                 && dynamic_cast<Function *>(paramEntry->type)->params.at(counter)->typeValue == TYPE_UNKNOWN) {

                                    CustomType *tempCT = paramEntry->params.at(counter)->type;
                                    tempCT->~CustomType();

                                    // Create a new object in the same space.
                                    if (exprEntry->params.at(counter)->type->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_ARRAY && exprEntry->params.at(counter)->type->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                                    else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FUNC) {
                                        tempCT = new (tempCT) Function(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->outputType);

                                        long unsigned int index = 0;
                                        while (index < dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.size())
                                            dynamic_cast<Function *>(tempCT)->params.push_back(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.at(index++));
                                    }

                                    // dynamic_cast<Function *>(paramEntry->type)->params.at(counter) = paramEntry->params.at(counter)->type;
                                }

                                /* type check */
                                if (paramEntry->params.at(counter)->type->typeValue != exprEntry->params.at(counter)->type->typeValue) {
                                    semError = true;
                                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                    Error *err = new TypeMismatch(paramEntry->params.at(counter)->type, exprEntry->params.at(counter)->type);
                                    err->printError();
                                }

                                counter++;
                            }

                            dynamic_cast<Function *>(paramEntry->type)->outputType = dynamic_cast<Function *>(exprEntry->type)->outputType;
                        }

                        if (tempEntry->params.at(i)->type->typeValue == TYPE_REF || tempExprGen->getType()->typeValue == TYPE_REF) {
                            std::pair <CustomType *, int> pairExpr1, pairExpr2;
                            pairExpr1 = getRefFinalType(tempEntry->params.at(i)->type);
                            pairExpr2 = getRefFinalType(tempExprGen->getType());
                            /* same but for Reference(Unknown()) */
                            if (pairExpr1.first->typeValue == TYPE_UNKNOWN && pairExpr2.first->typeValue == TYPE_UNKNOWN) {
                                /* Warning polymorphic value */
                                int levelsDiff = (pairExpr1.second > pairExpr2.second) ? pairExpr1.second - pairExpr2.second : pairExpr2.second - pairExpr1.second;
                                CustomType *t1 = pairExpr1.first;
                                CustomType *t2 = pairExpr2.first;
                                if (pairExpr1.second > pairExpr2.second) {
                                    while (levelsDiff > 0) {
                                        t2 = new Reference(new Unknown());
                                        t2 = t2->ofType;
                                        levelsDiff--;
                                    }
                                }
                                if (pairExpr1.second < pairExpr2.second) {
                                    while (levelsDiff > 0) {
                                        t1 = new Reference(new Unknown());
                                        t1 = t1->ofType;
                                        levelsDiff--;
                                    }
                                }
                            }
                            /* Check if either given param is of unknown type or function param is of unknown type - Type Inference */
                            else if (pairExpr1.first->typeValue == TYPE_UNKNOWN) {
                                if (pairExpr1.second > pairExpr2.second) {
                                    /* Print Error - type mismatch */

                                    /* keep the depth of expr2 in order to know how much to traverse
                                       into expr1 types and print the error correctly */
                                    int pairExpr2Depth = pairExpr2.second;
                                    CustomType *traverseType = tempEntry->params.at(i)->type;
                                    while (pairExpr2Depth != 1) { traverseType = traverseType->ofType; pairExpr2Depth--; }

                                    semError = true;
                                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                    Error *err = new TypeMismatch(traverseType, pairExpr2.first);
                                    err->printError();
                                }
                                else tempEntry->params.at(i)->type = tempExprGen->getType();
                            }
                            else if (pairExpr2.first->typeValue == TYPE_UNKNOWN) {
                                if (pairExpr1.second < pairExpr2.second) {
                                    /* Print Error - type mismatch */

                                    /* keep the depth of expr1 in order to know how much to traverse
                                       into expr2 types and print the error correctly */
                                    int pairExpr1Depth = pairExpr1.second;
                                    CustomType *traverseType = tempExprGen->getType();
                                    while (pairExpr1Depth != 1) { traverseType = traverseType->ofType; pairExpr1Depth--; }

                                    semError = true;
                                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                    Error *err = new TypeMismatch(traverseType, pairExpr1.first);
                                    err->printError();
                                }
                                else {
                                    tempExprGen->setType(tempEntry->params.at(i)->type);
                                    SymbolEntry *se = dynamic_cast<Id*>(tempExprGen->getExpr())->sem_getExprObj();
                                    se->type = tempExprGen->getType();
                                }
                            }
                            /* Check ith param given that has the same type as the ith param of the function */
                            else if (pairExpr1.first->typeValue != pairExpr2.first->typeValue || pairExpr1.second != pairExpr2.second ) {
                                /* Print Error - type mismatch */

                                int pairExprDepth;
                                CustomType *traverseType;

                                /* keep the depth of the expr that has the smallest depth
                                    in order to know how much to traverse
                                    into the other expr types and print the error correctly */
                                if (pairExpr1.second < pairExpr2.second) {
                                    pairExprDepth = pairExpr1.second;
                                    traverseType = tempExprGen->getType();
                                }
                                else {
                                    pairExprDepth = pairExpr2.second;
                                    traverseType = tempEntry->params.at(i)->type;
                                }

                                while (pairExprDepth != 1) { traverseType = traverseType->ofType; pairExprDepth--; }

                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;

                                Error *err;
                                if (pairExpr1.second < pairExpr2.second) err = new TypeMismatch(pairExpr1.first, traverseType);
                                else err = new TypeMismatch(traverseType, pairExpr2.first);
                                err->printError();
                            }
                        }
                    }
                    int extraParams = 0;
                    CustomType *lastTypeMore;
                    CustomType *lastTypeLess;
                    std::vector<CustomType *> params;
                    if (i < tempEntry->params.size()) {
                        lastTypeLess = tempEntry->params.at(i)->type;
                        params.push_back(lastTypeLess);
                    }
                    if (tempExprGen != nullptr) {
                        lastTypeMore = tempEntry->params.back()->type;
                        params.push_back(lastTypeMore);
                    }
                    /* given params are more than params in function -> Go through extra given params types */
                    while (tempExprGen != nullptr) {
                        /*  Print Error
                            type mismatch in expression,
                            mismatch in function application,
                            impossible to unify outputType with tempExprGen->expr->getType()->typeValue [int -> int -> int -> int] -> none
                        */
                        extraParams++;
                        params.push_back(tempExprGen->getType());
                        tempExprGen = tempExprGen->getNext();
                    }
                    /* params in function are more than given params -> Go through extra function params types */
                    while (i < tempEntry->params.size()) {
                        /*  Print Error
                            type mismatch in expression,
                            partial function application,
                            offending type is tempEntry->params.at(i)->type->typeValue [@12 -> @13 -> @14] -> int
                        */
                        extraParams--;
                        params.push_back(tempEntry->params.at(i)->type);
                        i++;
                    }
                    /* Params given are more than expected */
                    if (extraParams > 0) {
                        CustomType *newFunc = new Function(new Unknown());
                        for (int j = 0; j < extraParams+1; j++) {
                            if (j == extraParams) dynamic_cast<Function *>(newFunc)->outputType = params.at(j);
                            else dynamic_cast<Function *>(newFunc)->params.push_back(params.at(j));
                        }
                        /* Print Error */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(tempEntry->params.back()->type, newFunc);
                        err->printError();
                    }
                    /* Params given are less than expected */
                    if (extraParams < 0) {
                        CustomType *newFunc = new Function(new Unknown());
                        for (int j = 0; j < -(extraParams-1); j++) {
                            if (j == -extraParams) dynamic_cast<Function *>(newFunc)->outputType = params.at(j);
                            else dynamic_cast<Function *>(newFunc)->params.push_back(params.at(j));
                        }
                        /* Print Error */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err;
                        if (tempExprGen != nullptr) err = new TypeMismatch(newFunc, tempExprGen->getType());
                        else err = new TypeMismatch(newFunc, expr->getType());
                        err->printError();
                    }
                }
                /* edge case - when calling a function with exactly 1 param but more are required */
                else {
                    if (tempEntry->params.size() > 1) {
                        std::vector<CustomType *> params;
                        CustomType *newFunc = new Function(new Unknown());
                        for (long unsigned int j = 0; j < tempEntry->params.size(); j++) {
                            if (j == tempEntry->params.size()-1) dynamic_cast<Function *>(newFunc)->outputType = tempEntry->params.at(j)->type;
                            else dynamic_cast<Function *>(newFunc)->params.push_back(tempEntry->params.at(j)->type);
                        }
                        /* Print Error */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err;
                        err = new TypeMismatch(newFunc, expr->getType());
                        err->printError();
                    }
                }
            }
            else {
                /* type should be set to unknown because when returned to the class
                   that triggered this sem this->type will be null */
                /* Print Error First Occurence */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new FirstOccurence(name);
                err->printError();

                tempEntry = new SymbolEntry(name, new Unknown());
                /* Now Unknown is recognized as None */
                tempEntry->type->size = 0;
                this->type = tempEntry->type;
                tempEntry->entryType = ENTRY_TEMP;
                st.insert(name, tempEntry);
                this->sem();
            }
        }
    }

    virtual llvm::Value* compile() const override {
        if (expr == nullptr && exprGen == nullptr) {
            SymbolEntry *se = currPseudoScope->lookup(name, pseudoST.getSize());
            if (se != nullptr) {
                return se->Value;
            }
        }
        else {
            std::vector<llvm::Value *> args;
            llvm::Value *v = expr->compile();

            args.push_back(v);
            ExprGen *currExpr = exprGen;
            while (currExpr != nullptr) {
                v = currExpr->compile();
                args.push_back(v);
                currExpr = currExpr->getNext();
            }
            /* print_ functions return unit, therefore don't return Builder.CreateCall(...),
            but instead return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
            */
            // might need to do the same with custom functions
            if (!name.compare("print_int")) Builder.CreateCall(TheWriteInteger, args);
            else if (!name.compare("print_bool")) Builder.CreateCall(TheWriteBoolean, args);
            else if (!name.compare("print_char")) Builder.CreateCall(TheWriteChar, args);
            else if (!name.compare("print_float")) Builder.CreateCall(TheWriteReal, args);
            else if (!name.compare("print_string")) Builder.CreateCall(TheWriteString,Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(0),std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")));
            else if (!name.compare("read_int")) return Builder.CreateCall(TheReadInteger);
            else if (!name.compare("read_bool")) return Builder.CreateCall(TheReadBoolean);
            else if (!name.compare("read_char")) return Builder.CreateCall(TheReadChar);
            else if (!name.compare("read_float")) return Builder.CreateCall(TheReadReal);
            else if (!name.compare("read_string")) return Builder.CreateCall(TheReadString);
            else if (!name.compare("pi")) return Builder.CreateCall(ThePi);
            else if (!name.compare("int_of_float")) return Builder.CreateCall(TheIntOfFloat, args);
            else if (!name.compare("int_of_char")) return Builder.CreateCall(TheIntOfChar, args);
            else if (!name.compare("char_of_int")) return Builder.CreateCall(TheCharOfInt, args);
            else if (!name.compare("strlen")) return Builder.CreateCall(TheStringLength, Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(0), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")));
            else if (!name.compare("strcmp")) return Builder.CreateCall(TheStringCompare, std::vector<llvm::Value *> { Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(0), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")), Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(1), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")) });
            else if (!name.compare("strcpy")) return Builder.CreateCall(TheStringCopy, std::vector<llvm::Value *> { Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(0), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")), Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(1), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")) });
            else if (!name.compare("strcat")) return Builder.CreateCall(TheStringConcat, std::vector<llvm::Value *> { Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(0), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")), Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_String_1"), args.at(1), std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringPtr")) });
            else return Builder.CreateCall(TheModule->getFunction(name), args);

            return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
        }

        return nullptr;
    }

private:
    std::string name;
    Expr *expr;
    ExprGen *exprGen;
};

class PatternId : public Pattern {
public:
    PatternId(std::string id): name(id) {}

    virtual void printOn(std::ostream &out) const override {
        out << "PatternId(" << name <<")";
    }

    std::string getName() override { return name; }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(name); }

    virtual void sem() override {
        // /* lookup for variable if exists, if yes it's a duplicate error otherwise create  */
        SymbolEntry *tempEntry = st.lookup(name, true);
        if (tempEntry != nullptr) {
            semError = true;
            this->type = new Unknown();
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new DuplicateEntry(name, true, true);
            err->printError();
        }
        else {
            CustomType *ct = new Unknown();
            st.insert(name, ct, ENTRY_VARIABLE);
            this->type = ct;
        }
    }

    virtual llvm::Value* compile() const override {
        pseudoST.incrSize();
        return c1(true);
    }

protected:
    std::string name;
};

class PatternGen : public Pattern {
public:
    PatternGen(Pattern *p, PatternGen *pg ): pattern(p), patternGen(pg) {}

    virtual void printOn(std::ostream &out) const override {
        if (patternGen == nullptr) {
            out << "PatternGen("; pattern->printOn(out); out << ")" ;
        }
        else {
            out << "PatternGen("; pattern->printOn(out); out << ", "; patternGen->printOn(out); out << ")";
        }
    }

    virtual SymbolEntry *sem_getExprObj() override { return pattern->sem_getExprObj(); }

    PatternGen *getNext() { return patternGen; }

    virtual void sem() override {
        pattern->sem();
        this->type = pattern->getType();
        if (patternGen != nullptr) patternGen->sem();
    }

    virtual llvm::Value* compile() const override {
        pattern->setMatchExprV(matchExprV);
        pattern->setNextClauseBlock(nextClauseBlock);
        return pattern->compile();
    }

private:
    Pattern *pattern;
    PatternGen *patternGen;
};

class PatternConstr : public Pattern {
public:
    PatternConstr(std::string id, PatternGen *pg): Id(id), patternGen(pg) {}

    virtual void printOn(std::ostream &out) const override {
        if (patternGen == nullptr) {
            out << "PatternConstrId(" << Id << ")";
        }
        else {
            out << "PatternConstrId(" << Id << ", "; patternGen->printOn(out); out << ")";
        }
    }

    virtual void sem() override {
        SymbolEntry *tempEntry = st.lookup(Id);
        if (tempEntry == nullptr) {
            /* Print Error - first Occurence */
            semError = true;
            this->type = new Unknown();
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new FirstOccurence(Id);
            err->printError();
        }
        // else this->type = tempEntry->type;
        //CHANGE: make type CustomType instead of CustomId - TYPE_ID exists only in SymbolEntries
        else this->type = tempEntry->params.front()->type;
        if (patternGen != nullptr) {
            patternGen->sem();
            if (tempEntry != nullptr) {
                PatternGen *tempPatternGen = patternGen;
                int index = 0;
                SymbolEntry *patternEntry;
                while (tempPatternGen != nullptr) {

                    /* get SymbolEntry of given param, if it's nullptr, means it's a PatternId, therefore do type inference */
                    patternEntry = tempPatternGen->sem_getExprObj();
                    if (patternEntry != nullptr && patternEntry->type->typeValue == TYPE_UNKNOWN) {
                        patternEntry->type = dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index);
                        if (patternEntry->type->typeValue == TYPE_FUNC) {
                            for(auto param : patternEntry->type->params)
                                patternEntry->params.push_back(new SymbolEntry(param));
                        }
                    }
                    // type checks
                    // type check that it's the same base type in case it's another constructor
                    if (tempPatternGen->getType()->typeValue == TYPE_CUSTOM && !tempPatternGen->sem_getExprObj()->params.empty()) {
                        if (tempPatternGen->sem_getExprObj()->params.front()->id != dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index)->name) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempEntry->params.front()->type, tempPatternGen->sem_getExprObj()->params.front()->type);
                            err->printError();
                        }
                    }
                    // type check for everything else
                    else if (tempPatternGen->getType()->typeValue != TYPE_UNKNOWN && tempPatternGen->getType()->typeValue != dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index)->typeValue) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        std::vector<CustomType *> expectedType;
                        expectedType.push_back(dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index));
                        Error *err = new Expectation(expectedType, tempPatternGen->getType());
                        err->printError();
                    }
                    tempPatternGen = tempPatternGen->getNext();
                    index++;
                }
            }
        }
    }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(Id); }

    virtual llvm::Value* compile() const override {

        SymbolEntry *se = currPseudoScope->lookup(Id, pseudoST.getSize());

        int patternConstr_tag;
        for (long unsigned int i = 0; i < se->params.front()->params.size(); i++) {
            if (se == se->params.front()->params.at(i)) patternConstr_tag = i;
        }
        
        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *EqConstrsBlock = llvm::BasicBlock::Create(TheContext);

        llvm::Value *matchExprV_tag = Builder.CreateLoad(Builder.CreateGEP(matchExprV, { c32(0), c32(0) }));
        llvm::Value *comparison = Builder.CreateICmpEQ(c32(patternConstr_tag), matchExprV_tag);

        Builder.CreateCondBr(comparison, EqConstrsBlock, nextClauseBlock);

        TheFunction->getBasicBlockList().push_back(EqConstrsBlock);
        Builder.SetInsertPoint(EqConstrsBlock);

        llvm::Value *matchExprV_casted = Builder.CreatePointerCast(matchExprV, se->LLVMType->getPointerTo());

        llvm::Value *matched = c1(true);
        PatternGen *tempPatternGen = patternGen;
        int index = 0;
        while (tempPatternGen != nullptr) {
            llvm::Value *temp = Builder.CreateLoad(Builder.CreateGEP(matchExprV_casted, { c32(0), c32(++index) }));

            tempPatternGen->setMatchExprV(temp);
            tempPatternGen->setNextClauseBlock(nextClauseBlock);
            temp = tempPatternGen->compile();

            matched = Builder.CreateAnd(matched, temp);

            tempPatternGen = tempPatternGen->getNext();
        }

        return matched;

    }

protected:
    std::string Id;
    PatternGen *patternGen;
};

class Clause : public Expr {
public:
    Clause(Pattern *p, Expr *e): pattern(p), expr(e) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Clause("; pattern->printOn(out); out << ", "; expr->printOn(out); out <<")";

    }

    Pattern *getPattern() { return pattern; }

    virtual SymbolEntry *sem_getExprObj() override { return pattern->sem_getExprObj(); }

    Expr *getExpr() { return expr; }

    void setType(CustomType *t) { this->type = t; }

    virtual void sem() override {
        pattern->sem();
        expr->sem();
        this->type = expr->getType();
    }

    llvm::Value* patternCompile() { return pattern->compile(); }

    virtual llvm::Value* compile() const override {
        return expr->compile();
    }

private:
    Pattern *pattern;
    Expr *expr;
};

class BarClauseGen : public Expr {
public:
    BarClauseGen(Clause *c, BarClauseGen *bcg): clause(c), barClauseGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        if (barClauseGen == nullptr) {
            out << "BarClauseGen("; clause->printOn(out); out <<")";
        }
        else {
            out << "BarClauseGen("; clause->printOn(out); out << ", "; barClauseGen->printOn(out); out <<")";
        }

    }

    Clause *getClause() { return clause; }

    BarClauseGen *getBarClauseGen() { return barClauseGen; }

    virtual void sem() override {
        st.openScope();
        clause->sem();
        st.closeScope();
        this->type = clause->getType();
        if (barClauseGen != nullptr) barClauseGen->sem();
    }

    /* intentionally left empty =) */
    virtual llvm::Value* compile() const override {
        return nullptr;
    }

private:
    Clause *clause;
    BarClauseGen *barClauseGen;
};

class Match : public Expr {
public:
    Match(Expr *e, Clause *c, BarClauseGen *bcg): expr(e), clause(c), barClauseGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Match("; expr->printOn(out); out << ", "; clause->printOn(out);
        if (barClauseGen != nullptr) { out << ", "; barClauseGen->printOn(out); }
        out << ")";

    }

    virtual SymbolEntry *sem_getExprObj() override { return clause->getExpr()->sem_getExprObj(); }

    virtual void sem() override {
        st.openScope();
        expr->sem();
        st.openScope();
        clause->sem();
        st.closeScope();

        /* if type of expression is unknown, set it to type of clause */
        SymbolEntry *exprEntry = expr->sem_getExprObj();
        if (expr->getType()->typeValue == TYPE_UNKNOWN && exprEntry->type->typeValue == TYPE_UNKNOWN
         && clause->getPattern()->getType() != nullptr && clause->getPattern()->getType()->typeValue != TYPE_UNKNOWN) {
            exprEntry->type = clause->getPattern()->getType();
        }
        /* might need to remove below if and assign this->type even if clause doesn't have a type yet */
        if (clause->getType() != nullptr) {
            if (this->type != nullptr && this->type->typeValue == TYPE_UNKNOWN) {
                // Destroy the object but leave the space allocated.
                CustomType *tempCT = this->type;
                std::string ctName;
                if (!clause->getType()->name.empty()) ctName = clause->getType()->name;
                tempCT->~CustomType();
                std::cout <<"In Match changing this->type to " <<ctName <<std::endl; std::cout.flush();

                // Create a new object in the same space.
                if (clause->getType()->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                else if (clause->getType()->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                else if (clause->getType()->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                else if (clause->getType()->typeValue == TYPE_ARRAY && clause->getType()->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                else if (clause->getType()->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                else if (clause->getType()->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                else if (clause->getType()->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                else if (clause->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
            }
            else this->type = clause->getType();
        }

        /* pointer to get the type of first clause expr (will be used as prev pointer to compare with the next clause exprs) */
        SymbolEntry *prevSE = clause->getExpr()->sem_getExprObj();
        CustomType *prev = clause->getType();
        
        if (barClauseGen != nullptr) {
            barClauseGen->sem();
            /* pointer to iterate through Clauses (barClauseGen) */
            BarClauseGen *tempBarClauseGen = barClauseGen;
            while (tempBarClauseGen != nullptr) {
                CustomType *clausePatternType = tempBarClauseGen->getClause()->getPattern()->getType();

                if (exprEntry->type->typeValue == TYPE_CUSTOM) {
                    if (prev->typeValue == TYPE_UNKNOWN && tempBarClauseGen->getType()->typeValue != TYPE_UNKNOWN) {
                        /* Change previous clause type according to current clause type */
                        if (prev->typeValue == TYPE_UNKNOWN) {
                            prev = tempBarClauseGen->getType();
                        }
                        /* Change previous clause SymbolEntry (if it exists) according to current clause type */
                        if (prevSE != nullptr && prevSE->type != nullptr) {
                            if (prevSE->type->typeValue == TYPE_UNKNOWN) {
                                prevSE->type = tempBarClauseGen->getType();
                            }
                        }
                    }
                    /* type inference for clause return type - if current is unknown */
                    if (prev->typeValue != TYPE_UNKNOWN && tempBarClauseGen->getType()->typeValue == TYPE_UNKNOWN) {

                        // might need to implement destructor
                        tempBarClauseGen->setType(prev);

                        SymbolEntry *tempSE = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj();

                        /* in case that the clause return type is the function itself */
                        if (tempSE != nullptr)
                            if (tempSE->type->typeValue == TYPE_FUNC)
                                dynamic_cast<Function *>(tempSE->type)->outputType = prev;
                    }

                    /* type check all clause (need to be the same type) */
                    if (prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                        /* if both are customtypes */
                        if (prev->typeValue == TYPE_CUSTOM && tempBarClauseGen->getType()->typeValue == TYPE_CUSTOM
                        && prev->name != tempBarClauseGen->getType()->name) {
                            /* Print Error - cannot unify a with b */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                        /* if prev is unrelated to types */
                        else if ((prev->typeValue != TYPE_ID && prev->typeValue != TYPE_CUSTOM) && prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                            /* Print Error - cannot unify a with b */
                            /* if current clause is a constructor while prev wasn't */
                            if (tempBarClauseGen->getType()->typeValue == TYPE_ID && tempBarClauseGen->getClause()->sem_getExprObj() != nullptr) {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(prev, tempBarClauseGen->getClause()->sem_getExprObj()->params.front()->type);
                                err->printError();

                            }
                            /* if both clauses aren't constructors */
                            else {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                                err->printError();
                            }
                        }
                        /* if current is unrelated to types */
                        else if ((tempBarClauseGen->getType()->typeValue != TYPE_ID && tempBarClauseGen->getType()->typeValue != TYPE_CUSTOM) && prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                            /* Print Error - cannot unify a with b */
                            /* if current clause is a constructor while prev wasn't */
                            if (prev->typeValue == TYPE_ID && prevSE != nullptr) {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(dynamic_cast<CustomType *>(prevSE->params.front()->type), tempBarClauseGen->getType());
                                err->printError();

                            }
                            /* if both clauses aren't constructors */
                            else {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                                err->printError();
                            }
                        }
                    }
                }
                /* in case that expr does not have TYPE_ID or TYPE_CUSTOM */
                else {

                    if (exprEntry->type->typeValue == TYPE_ARRAY && exprEntry->type->ofType->typeValue == TYPE_UNKNOWN) {
                        exprEntry->type->ofType = clause->getPattern()->sem_getExprObj()->params.at(0)->type;
                    }
                    else if (((exprEntry->type->typeValue == TYPE_UNKNOWN && clausePatternType->typeValue != TYPE_UNKNOWN)
                     || (exprEntry->type->typeValue != TYPE_UNKNOWN && clausePatternType->typeValue == TYPE_CUSTOM))
                      && exprEntry->type->typeValue != TYPE_FUNC && exprEntry->type->typeValue != TYPE_ARRAY) {
                        exprEntry->type = clause->getPattern()->getType();
                     }
                }

                if (this->type != nullptr && this->type->typeValue == TYPE_UNKNOWN) {
                    // Destroy the object but leave the space allocated.
                    CustomType *tempCT = this->type;
                    std::string ctName;
                    if (!tempCT->name.empty()) ctName = prev->name;
                    tempCT->~CustomType();

                    // Create a new object in the same space.
                    if (prev->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                    else if (prev->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                    else if (prev->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                    else if (prev->typeValue == TYPE_ARRAY && prev->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                    else if (prev->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                    else if (prev->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                    else if (prev->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                    else if (prev->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
                }
                else this->type = prev;

                /* move pointers */
                prev = tempBarClauseGen->getType();
                prevSE = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj();
                tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
            }

            if (exprEntry->type->typeValue != TYPE_UNKNOWN) {

                /* type inference for first clause object */
                if (clause->sem_getExprObj() != nullptr && clause->sem_getExprObj()->type->typeValue == TYPE_UNKNOWN) {
                    SymbolEntry *firstClauseObj = clause->sem_getExprObj();
                    firstClauseObj->type = exprEntry->type;
                }

                /* if clause returns something other than Unknown or CustomType */
                if (clause->getPattern()->getType()->typeValue != TYPE_CUSTOM && clause->getPattern()->getType()->typeValue != TYPE_UNKNOWN) {
                    /* if expr is something other than CustomType -> type mismatch CustomType(expr) with CustomType(clause) */
                    if (exprEntry->type->typeValue == TYPE_CUSTOM) {
                        /* Print Error - cannot unify exprEntry->type->id with clause->getPattern()->getType()->typeValue */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                        err->printError();
                    }
                    else if (exprEntry->type->typeValue != clause->getPattern()->getType()->typeValue) {
                        /* Print Error - cannot unify exprEntry->type->typeValue with clause->getPattern()->getType()->typeValue */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                        err->printError();
                    }
                }

                tempBarClauseGen = barClauseGen;

                while (tempBarClauseGen != nullptr) {
                    /* type of pattern */
                    CustomType *clausePatternType = tempBarClauseGen->getClause()->getPattern()->getType();
                    /* SymbolEntry needed to change ret */
                    CustomType *clauseType = tempBarClauseGen->getClause()->getType();
                    /* exprEntry is a lowercase id */
                    if (exprEntry->type->typeValue != TYPE_CUSTOM) {
                        /* type check clause type != expr type */
                        if (clausePatternType->typeValue != TYPE_CUSTOM && clausePatternType->typeValue != TYPE_UNKNOWN)
                            if (exprEntry->type->typeValue != clausePatternType->typeValue) {
                                /* Print Error - cannot unify a with b */
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(exprEntry->type, clausePatternType);
                                err->printError();
                            }
                    }
                    else if (clausePatternType->typeValue != TYPE_CUSTOM && clausePatternType->typeValue != TYPE_UNKNOWN) {
                        /* Print Error - cannot unify exprEntry->type->id with clause->getPattern()->getType()->typeValue */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                        err->printError();
                    }

                    /* move pointers */
                    tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
                    /* type inference for return type of clause */
                    if (clauseType->typeValue == TYPE_UNKNOWN) clauseType = exprEntry->type;
                }
            }
        }
        st.closeScope();
    }

    virtual llvm::Value* compile() const override {

        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
        
        /* get expr compile to be matched */
        currPseudoScope = currPseudoScope->getNext();
        llvm::Value *matchExpr = expr->compile();

        /* vector to keep all clauses in order to iterate through it */
        std::vector<Clause *> clauses;
        clauses.push_back(clause);
        
        if (barClauseGen != nullptr) {
            BarClauseGen *tempBCG = barClauseGen;
            while (tempBCG != nullptr) {
                clauses.push_back(tempBCG->getClause());
                tempBCG = tempBCG->getBarClauseGen();
            }
        }

        /* create necessary blocks and block vectors */
        std::vector<llvm::BasicBlock *> clausesBlocks;
        std::vector<llvm::Value *> clausesValues;
        llvm::BasicBlock *SuccessBlock;
        llvm::BasicBlock *NextClauseBlock = llvm::BasicBlock::Create(TheContext);
        llvm::BasicBlock *FinishBlock = llvm::BasicBlock::Create(TheContext);

        Builder.CreateBr(NextClauseBlock);

        for (auto clause : clauses) {

            /* move to next clause block */
            TheFunction->getBasicBlockList().push_back(NextClauseBlock);
            Builder.SetInsertPoint(NextClauseBlock);

            /* create next and success block of clause */
            NextClauseBlock = llvm::BasicBlock::Create(TheContext);
            SuccessBlock = llvm::BasicBlock::Create(TheContext);

            /* move scope as each clause opens a scope */
            currPseudoScope = currPseudoScope->getNext();

            /* set to each clause the expression they are trying to match */
            clause->getPattern()->setMatchExprV(matchExpr);
            /* set to each clause their next clause block */
            clause->getPattern()->setNextClauseBlock(NextClauseBlock);
            
            /* branch in case clause pattern matches the expr pattern */
            Builder.CreateCondBr(clause->patternCompile(), SuccessBlock, NextClauseBlock);

            /* if clause pattern matches the expr pattern */
            TheFunction->getBasicBlockList().push_back(SuccessBlock);
            Builder.SetInsertPoint(SuccessBlock);

            clausesValues.push_back(clause->compile());

            /* move scope back every time a clause finishes */
            currPseudoScope = currPseudoScope->getPrev();

            /* block needed for phi node */
            clausesBlocks.push_back(Builder.GetInsertBlock());

            Builder.CreateBr(FinishBlock);
        }

        /* case that no clause pattern matched the expr pattern */
        TheFunction->getBasicBlockList().push_back(NextClauseBlock);
        Builder.SetInsertPoint(NextClauseBlock);

        Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Match Failure\n")) });
        Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });

        Builder.CreateBr(NextClauseBlock);

        /* finish of match */
        TheFunction->getBasicBlockList().push_back(FinishBlock);
        Builder.SetInsertPoint(FinishBlock);

        llvm::Type *returnTy = clause->getType()->getLLVMType();
        llvm::PHINode *v = Builder.CreatePHI(returnTy, clauses.size());
        for (long unsigned int i = 0; i < clauses.size(); i++) v->addIncoming(clausesValues[i], clausesBlocks[i]);

        currPseudoScope = currPseudoScope->getPrev();

        return v;
    }

private:
    Expr *expr;
    Clause *clause;
    BarClauseGen *barClauseGen;
};

class For : public Expr, public Block {
public:
    For(char *id, Expr *s, Expr *end, Expr *e, bool isAscending):
        id(id), start(s), end(end), expr(e), ascending(isAscending) {}

    virtual void printOn(std::ostream &out) const override {

        out << "For("<< id <<", "; start->printOn(out); out <<", "; end->printOn(out); out <<", "; expr->printOn(out); out <<", " << ascending <<")";

    }

    virtual void sem() override {
        /* Open scope, go to BinOp and save the variable */
        st.openScope();
        st.insert(id, new Integer(), ENTRY_VARIABLE);
        start->sem();
        if (start->getType()->typeValue != TYPE_INT) {
            /* Print Error */
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Integer());
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << start->YYLTYPE.first_line << ", Characters " << start->YYLTYPE.first_column << " - " << start->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, start->getType());
            err->printError();
        }
        end->sem();
        if (end->getType()->typeValue != TYPE_INT) {
            /* Print Error */
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Integer());
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << end->YYLTYPE.first_line << ", Characters " << end->YYLTYPE.first_column << " - " << end->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, end->getType());
            err->printError();
        }
        /* if everything ok then proceed */
        expr->sem();
        this->type = expr->getType();
        st.closeScope();
    }

    virtual llvm::Value* compile() const override {
        /* compile start */
        llvm::Value *startValue = start->compile();
        if (startValue == nullptr) return nullptr;

        // Make the new basic block for the loop header, inserting after current
        // block.
        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *PreheaderBB = Builder.GetInsertBlock();
        llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

        // Insert an explicit fall through from the current block to the LoopBB.
        Builder.CreateBr(LoopBB);

        // Start insertion in LoopBB.
        Builder.SetInsertPoint(LoopBB);

        // Start the PHI node with an entry for Start.
        llvm::PHINode *Variable =
        Builder.CreatePHI(llvm::Type::getInt32Ty(TheContext), 2, id);
        Variable->addIncoming(startValue, PreheaderBB);

        // increase size of pseudoST for correct lookup (newly added var of For)
        pseudoST.incrSize();
        // fetch from SymbolTable
        currPseudoScope = currPseudoScope->getNext();
        SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
        se->Value = Variable;

        // Emit the body of the loop.  This, like any other expr, can change the
        // current BB. Note that we ignore the value computed by the body, but don't
        // allow an error.
        if (!expr->compile()) return nullptr;

        // Emit the step value. Not supported, use 1.
        llvm::Value *stepValue = nullptr;
        stepValue = c32(1);

        llvm::Value *nextVar = nullptr;
        if (ascending) nextVar = Builder.CreateAdd(Variable, stepValue, "nextvar");
        else nextVar = Builder.CreateSub(Variable, stepValue, "nextvar");

        // Compute the end condition.
        llvm::Value *endVar = end->compile();
        if (!endVar) return nullptr;

        llvm::Value *endCond = Builder.CreateICmpNE(nextVar, endVar);

        // Create the "after loop" block and insert it.
        llvm::BasicBlock *LoopEndBB = Builder.GetInsertBlock();
        llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext, "afterloop", TheFunction);

        // Insert the conditional branch into the end of LoopEndBB.
        Builder.CreateCondBr(endCond, LoopBB, AfterBB);

        // Any new code will be inserted in AfterBB.
        Builder.SetInsertPoint(AfterBB);

        // Add a new entry to the PHI node for the backedge.
        Variable->addIncoming(nextVar, LoopEndBB);

        // close Scope
        currPseudoScope = currPseudoScope->getPrev();

        // for expr always returns 0.
        return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
    }

private:
    char *id;
    Expr *start;
    Expr *end;
    Expr *expr;
    bool ascending;
};

class While : public Expr, public Block {
public:
    While(Expr *lc, Expr *e): loopCondition(lc), expr(e) {}

    virtual void printOn(std::ostream &out) const override {

        out << "While("; loopCondition->printOn(out); out <<", "; expr->printOn(out); out <<")";
    }

    virtual void sem() override {
        st.openScope();
        loopCondition->sem();
        if (loopCondition->getType()->typeValue != TYPE_BOOL) {
            /* Print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Boolean());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << loopCondition->YYLTYPE.first_line << ", Characters " << loopCondition->YYLTYPE.first_column << " - " << loopCondition->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, loopCondition->getType());
            err->printError();
        }
        expr->sem();
        this->type = expr->getType();
        st.closeScope();
    }

    virtual llvm::Value* compile() const override {
        currPseudoScope = currPseudoScope->getNext();
        llvm::Value *n = loopCondition->compile();
        llvm::BasicBlock *PrevBB = Builder.GetInsertBlock();
        llvm::Function *TheFunction = PrevBB->getParent();
        llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);
        llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(TheContext, "body", TheFunction);
        llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext, "endwhile", TheFunction);
        Builder.CreateBr(LoopBB);
        Builder.SetInsertPoint(LoopBB);
        llvm::PHINode *phi_iter = Builder.CreatePHI(i1, 2, "iter");
        phi_iter->addIncoming(n, PrevBB);
        llvm::Value *loop_cond = Builder.CreateICmpNE(phi_iter, c1(0), "loop_cond");
        Builder.CreateCondBr(loop_cond, BodyBB, AfterBB);
        Builder.SetInsertPoint(BodyBB);
        expr->compile();
        phi_iter->addIncoming(loopCondition->compile(), Builder.GetInsertBlock());
        Builder.CreateBr(LoopBB);
        Builder.SetInsertPoint(AfterBB);
        return nullptr;
    }

private:
    Expr *loopCondition;
    Expr *expr;
};

class If : public Expr, public Block {
public:
    If(Expr *c, Expr *e1, Expr *e2): condition(c), expr1(e1), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {

        if (expr2 == nullptr) {
            out << "If("; condition->printOn(out); out << ", "; expr1->printOn(out); out << ")";
        }
        else {
            out << "If("; condition->printOn(out); out << ", "; expr1->printOn(out); out << ", "; expr2->printOn(out); out << ")";
        }
    }

    // virtual SymbolEntry *sem_getExprObj() override { return expr1->sem_getExprObj(); }

    virtual void sem() override {
        condition->sem();
        /* type inference */
        if (condition->getType()->typeValue == TYPE_UNKNOWN && condition->sem_getExprObj() != nullptr) {
            CustomType *ct = new Boolean();
            condition->setType(ct);
            CustomType *temp = condition->sem_getExprObj()->type;
            while (temp->typeValue == TYPE_REF) temp = temp->ofType;
            temp = ct;
        }

        /* type check */
        if (condition->getType()->typeValue != TYPE_BOOL) {
            /* print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Boolean());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << condition->YYLTYPE.first_line << ", Characters " << condition->YYLTYPE.first_column << " - " << condition->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, condition->getType());
            err->printError();
        }
        expr1->sem();

        if (expr2 == nullptr) {
            if (expr1->getType()->typeValue == TYPE_UNKNOWN) {
                expr1->setType(new Unit());
            }
            else if (expr1->getType()->typeValue != TYPE_UNIT) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr1->YYLTYPE.first_line << ", Characters " << expr1->YYLTYPE.first_column << " - " << expr1->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType(), new Unit());
                err->printError();
            }
        }

        if (expr2 != nullptr) {
            expr2->sem();
            /* might need to reconsider expr1 and expr2 getType()->typeValue == TYPE_UNKNOWN */
            if (expr1->getType()->typeValue == TYPE_UNKNOWN) expr1->setType(expr2->getType());
            else if (expr2->getType()->typeValue == TYPE_UNKNOWN) expr2->setType(expr1->getType());
            /* expr1 and expr2 must be of same type */
            else if (expr1->getType()->typeValue != expr2->getType()->typeValue) {
                /* print Error */
                /* if expr1 is a constructor and expr2 is a custom type */
                if (expr1->getType()-> typeValue == TYPE_ID && expr2->getType()->typeValue == TYPE_CUSTOM
                && expr1->sem_getExprObj() != nullptr && expr1->sem_getExprObj()->params.front()->type->name != expr2->getType()->name) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(expr1->sem_getExprObj()->params.front()->type, expr2->getType());
                    err->printError();
                }
                /* if expr1 is a constructor and expr2 is anything *but* a custom type */
                else if (expr1->getType()-> typeValue == TYPE_ID && expr2->getType()->typeValue != TYPE_CUSTOM
                && expr1->sem_getExprObj() != nullptr) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(expr1->sem_getExprObj()->params.front()->type, expr2->getType());
                    err->printError();
                }
                /* if expr1 is not a constructor and expr2 is a custom type */
                else if (expr1->getType()-> typeValue != TYPE_ID && expr2->getType()->typeValue == TYPE_CUSTOM) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                    err->printError();
                }
                /* if both are unrelated to types */
                else if (expr1->getType()->typeValue != TYPE_ID && expr2->getType()->typeValue != TYPE_CUSTOM) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                    err->printError();
                }
            }
        }
        if (expr1->getType()->typeValue != TYPE_ID) this->type = expr1->getType();
        else {
            SymbolEntry *tempEntry = expr1->sem_getExprObj();
            this->type = tempEntry->params.front()->type;
        }

    }

    virtual llvm::Value* compile() const override {
        llvm::Value *v = condition->compile();
        llvm::Value *cond = Builder.CreateICmpNE(v, c1(false), "if_cond");
        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *ThenBB =
        llvm::BasicBlock::Create(TheContext, "then", TheFunction);
        llvm::BasicBlock *ElseBB =
        llvm::BasicBlock::Create(TheContext, "else", TheFunction);
        llvm::BasicBlock *AfterBB =
        llvm::BasicBlock::Create(TheContext, "endif", TheFunction);
        Builder.CreateCondBr(cond, ThenBB, ElseBB);
        Builder.SetInsertPoint(ThenBB);
        expr1->compile();
        Builder.CreateBr(AfterBB);
        Builder.SetInsertPoint(ElseBB);
        if (expr2 != nullptr) {
            expr2->compile();
        }
        Builder.CreateBr(AfterBB);
        Builder.SetInsertPoint(AfterBB);
        return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
    }

private:
    Expr *condition;
    Expr *expr1;
    Expr *expr2;
};

class Begin : public Expr, public Block {
public:
    Begin(Expr *e): expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "Begin("; expr->printOn(out); out <<")";
    }

    virtual void sem() override {
        std::cout <<"In Begin\n"; std::cout.flush();
        st.openScope();
        expr->sem();
        std::cout <<"In Begin after sem\n"; std::cout.flush();
        this->type = expr->getType();
        st.closeScope();
    }

    virtual llvm::Value* compile() const override {
        currPseudoScope = currPseudoScope->getNext();
        llvm::Value *v = expr->compile();
        currPseudoScope = currPseudoScope->getPrev();
        return v;
    }

private:
    Expr *expr;
};

class CommaExprGen : public Expr {
public:
    CommaExprGen(Expr *e, CommaExprGen *ceg): expr(e), commaExprGen(ceg) {}

    virtual void printOn(std::ostream &out) const override {

        if (commaExprGen == nullptr) {
            out << "CommaExprGen("; expr->printOn(out); out << ")";
        }
        else {
            out << "CommaExprGen("; expr->printOn(out); out << ", "; commaExprGen->printOn(out); out << ")";
        }

    }

    CommaExprGen *getNext() { return commaExprGen; }

    virtual void sem() override {
        expr->sem();
        /* type inference */
        if (getRefFinalType(expr->getType()).first->typeValue == TYPE_UNKNOWN) {
            expr->setType(new Integer());
            SymbolEntry *se = expr->sem_getExprObj();
            if (se != nullptr) {
                CustomType *finalType = se->type;
                while(finalType->ofType != nullptr) finalType = finalType->ofType;
                finalType = new Integer();
            }
        }
        /* type check */
        if (expr->getType()->typeValue != TYPE_INT) {
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new TypeMismatch(new Integer(), expr->getType());
            err->printError();
        }
        if (commaExprGen != nullptr) commaExprGen->sem();
    }

    virtual llvm::Value* compile() const override {
        return expr->compile();
    }

private:
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class Par : public Expr {
public:
    Par(std::string id, CustomType* t): id(id), type(t) {}

    virtual void printOn(std::ostream &out) const override {

        if (type == nullptr) {
            out << "Par(" << id << ")";
        }
        else {
            out << "Par(" << id << ", "; type->printOn(out); out << ")" ;
        }

    }

    virtual void sem() override {
        /* Params of a function are saved in a vector attribute (params) of the Symbol Entry of the function */
        SymbolEntry *tempEntry = st.getLastEntry();
        SymbolEntry *newParamEntry;
        if (type != nullptr) newParamEntry = new SymbolEntry(id, type);
        else newParamEntry = new SymbolEntry(id, new Unknown());
        if (newParamEntry->type->typeValue == TYPE_FUNC) {
            SymbolEntry *paramOfFunc;
            std::string name;
            int counter = 0;
            for (auto i : dynamic_cast<Function *>(newParamEntry->type)->params) {
                name = newParamEntry->id + "_param_" + std::to_string(counter++);
                paramOfFunc = new SymbolEntry(name, i);
                paramOfFunc->entryType = ENTRY_PARAMETER;
                newParamEntry->params.push_back(paramOfFunc);
            }
            reverse(newParamEntry->params.begin(), newParamEntry->params.end());
        }
        newParamEntry->entryType = ENTRY_PARAMETER;
        tempEntry->params.push_back(newParamEntry);
    }

    virtual llvm::Value* compile() const override {
        pseudoST.incrSize();

        SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
        if (se != nullptr) {
            if (se->type->typeValue == TYPE_REF) se->LLVMType = se->type->getLLVMType()->getPointerTo();
            else se->LLVMType = se->type->getLLVMType();

            if (getRefFinalType(se->type).first->typeValue == TYPE_UNKNOWN) {
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Warning at: Line " << YYLTYPE.first_line << ", Characters " << YYLTYPE.first_column << " - " << YYLTYPE.last_column << std::endl;
                Error *err = new Warning(id);
                err->printError();
            }
        }

        return nullptr;
    }

private:
    std::string id;
    CustomType *type;
};

class ParGen : public Expr {
public:
    ParGen(Par *p, ParGen *pg): par(p), parGen(pg) {}

    virtual void printOn(std::ostream &out) const override {

        if (parGen == nullptr) {
            out << "ParGen("; par->printOn(out); out << ")";
        }
        else {
            out << "ParGen("; par->printOn(out); out << ", "; parGen->printOn(out); out << ")";
        }

    }
    // int -> int -> int -> int
    // Function(Function(int, Function(int, Function(int, nullptr))), int)
    virtual void sem() override {
        /* Params of a function are saved in a vector attribute (params) of the Symbol Entry of the function */
        // deprecated
        // SymbolEntry *tempEntry = st.getLastEntry();
        // if (parGen != nullptr) dynamic_cast<Function*>(tempEntry->type)->outputType = new Function(new Unknown(), new Unknown());
        par->sem();
        // deprecated
        // dynamic_cast<Function*>(tempEntry->type)->inputType = st.getLastEntry()->type;
        if (parGen != nullptr) parGen->sem();
    }

    ParGen *getNext() { return parGen; }

    virtual llvm::Value* compile() const override {
        par->compile();
        if (parGen != nullptr) 
            parGen->compile();

        return nullptr;
    }

private:
    Par *par;
    ParGen *parGen;
};

class Def : public AST {
public:
    Def(std::string id, ParGen *pg, Expr *e, CustomType *t, CommaExprGen *ceg, bool isMutable):
        id(id), parGen(pg), expr(e), type(t), commaExprGen(ceg), mut(isMutable) {}

    virtual void printOn(std::ostream &out) const override {

        if (mut) { out << "MutableDef(" << id; }
        else { out << "Def(" << id; }
        if (parGen != nullptr) { out << ", "; parGen->printOn(out); }
        if (expr != nullptr) { out << ", "; expr->printOn(out); }
        if (type != nullptr) { out << ", "; type->printOn(out); }
        if (commaExprGen != nullptr) { out <<", "; commaExprGen->printOn(out); }
        out << ")";
    }

    virtual void sem() override {
        /* if def is a mutable variable/array */
        if (mut) {
            /* variable */
            if (expr == nullptr) {
                /* variable's type is given */
                if (type != nullptr) st.insert(id, new Reference(type), ENTRY_VARIABLE);
                /* variable's type is unknown */
                else st.insert(id, new Reference(new Unknown()), ENTRY_VARIABLE);
            }
            /* array */
            else {
                /* dimensions = -1 is temporary is updated in Let */
                /* array's type is given */
                if (type != nullptr) st.insert(id, new Array(type, -1), ENTRY_VARIABLE);
                /* array's type is unknown */
                else st.insert(id, new Array(new Unknown(), -1), ENTRY_VARIABLE);
            }
        }
        else {
            /* if def is a non mutable variable - constant */
            if (parGen == nullptr) {
                /* not null type */
                if (type != nullptr) {
                    st.insert(id, type, ENTRY_CONSTANT);
                }
                /* null type means that type is equal to expression's */
                else {
                    st.insert(id, new Unknown(), ENTRY_CONSTANT);
                }
            }
            /* if def is a function */
            else {
                /* if type of function (return type) is given */
                if (type != nullptr) st.insert(id, new Function(type), ENTRY_FUNCTION);
                /* if type of function (return type) is not given */
                else st.insert(id, new Function(new Unknown()), ENTRY_FUNCTION);
                /* get newly created function */
                SymbolEntry *funcEntry = st.getLastEntry();
                st.openScope();
                parGen->sem();
                /* after params are inserted into function's vector (params) also insert them into the new scope */
                for (auto i : funcEntry->params) {
                    funcEntry->type->params.push_back(i->type);
                    st.insert(i->id, i);
                }

                st.closeScope();
            }
        }
    }

    virtual llvm::Value* compile() const override {
        
        /* increase size of pseudoST for a new variable that was inserted */
        pseudoST.incrSize();

        if(!mut) {
            /* if def is a function */
            if (parGen != nullptr) {
                
                ParGen *tempParGen = parGen;
                while (tempParGen != nullptr) {
                    /* increase size of pseudoST for a new function param that was inserted */
                    pseudoST.incrSize();
                    tempParGen = tempParGen->getNext();
                }

                currPseudoScope = currPseudoScope->getNext();
                currPseudoScope = currPseudoScope->getPrev();
            }
        }
        return nullptr;
    }

    std::string id;
    ParGen *parGen;
    Expr *expr;
    CustomType *type;
    CommaExprGen *commaExprGen;
    bool mut;
};

class DefGen : public AST {
public:
    DefGen(Def *d, DefGen *dg): def(d), defGen(dg) {}

    virtual void printOn(std::ostream &out) const override {

        if (defGen == nullptr) {
            out << "DefGen("; def->printOn(out); out << ")" ;
        }
        else {
            out << "DefGen("; def->printOn(out); out << ", "; defGen->printOn(out); out << ")";
        }
    }

    virtual void sem() override {
        def->sem();
        if (defGen != nullptr) defGen->sem();
    }

    virtual llvm::Value* compile() const override {
        return 0;
    }

    Def *def;
    DefGen *defGen;
};

class Let : public Block {
public:
    Let(Def *d, DefGen *dg, bool isRec): def(d), defGen(dg), rec(isRec) {}

    virtual void printOn(std::ostream &out) const override {

        if (defGen == nullptr) {
            if (rec) {
                out << "LetRec("; def->printOn(out); out << ")";
            }
            else {
                out << "Let("; def->printOn(out); out << ")";
            }
        }
        else {
            if (rec) {
                out << "LetRec("; def->printOn(out); out << ", "; defGen->printOn(out); out << ")";
            }
            else {
                out << "Let("; def->printOn(out); out << ", "; defGen->printOn(out); out << ")";
            }
        }

    }

    virtual void sem() override {
        std::vector<SymbolEntry *> defsSE;
        SymbolEntry *tempSE;
        int index = 0;

        def->sem();
        defs.push_back(def);

        if (defGen != nullptr) defGen->sem();
        DefGen * tempDefGen = defGen;

        while (tempDefGen != nullptr) {
            defs.push_back(tempDefGen->def);
            tempDefGen = tempDefGen->defGen;
        }

        for (auto currDef : defs) {
            tempSE = st.lookup(currDef->id);
            defsSE.push_back(tempSE);
            if (!rec) tempSE->isVisible = false;
        }

        for (auto currDef : defs) {

            // st.printST();

            tempSE = defsSE.at(index++);

            /* if def is a mutable variable/array */
            if (currDef->mut) {
                /* variable */
                if (currDef->expr == nullptr) {}
                /* array */
                else {
                    currDef->expr->sem();
                    if (currDef->expr->getType()->typeValue != TYPE_INT) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << currDef->expr->YYLTYPE.first_line << ", Characters " << currDef->expr->YYLTYPE.first_column << " - " << currDef->expr->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(new Integer(), currDef->expr->getType());
                        err->printError();
                    }
                    if (currDef->commaExprGen != nullptr) currDef->commaExprGen->sem();

                    /* get dimensions by iterating commaExprGen "list" */
                    int dimensions = 1;
                    CommaExprGen *tempExpr = currDef->commaExprGen;
                    while (tempExpr != nullptr) {
                        dimensions++;
                        tempExpr = tempExpr->getNext();
                    }

                    tempSE->type->size = dimensions;
                }
            }
            else {
                /* if def is a non mutable variable - constant */
                if (currDef->parGen == nullptr) {
                    currDef->expr->sem();
                    /* not null type */
                    if (currDef->type != nullptr) {
                        /* check if type given is not same as expression's */
                        if (currDef->type->typeValue != currDef->expr->getType()->typeValue) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << " Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempSE->type, currDef->expr->getType());
                            err->printError();
                        }
                        tempSE->type = currDef->type;
                    }
                    /* null type means that type is equal to expression's */
                    else {

                        /* expression's type might be of type ref so we need to save the entry in st as entry variable and not constant (is mutable) */
                        tempSE->type = currDef->expr->getType();

                        if (currDef->expr->getType()->typeValue == TYPE_FUNC) {
                            SymbolEntry *paramOfFunc;
                            std::string name;
                            int counter = 0;
                            for (auto t : dynamic_cast<Function *>(currDef->expr->getType())->params) {
                                name = tempSE->id + "_param_" + std::to_string(counter++);
                                paramOfFunc = new SymbolEntry(name, t);
                                paramOfFunc->entryType = ENTRY_PARAMETER;
                                tempSE->params.push_back(paramOfFunc);
                            }
                        }

                        if (currDef->expr->getType()->typeValue != TYPE_REF) tempSE->entryType = ENTRY_CONSTANT;
                        else tempSE->entryType = ENTRY_VARIABLE;
                    }
                }
                /* if def is a function */
                else {
                    st.openScope();

                    for (auto param : tempSE->params) {
                        st.insert(param->id, param);
                        // dynamic_cast<Function *>(tempSE->type)->params.push_back(param->type);
                    }

                    if(tempSE->isVisible) {
                        currDef->expr->setType(new Unknown());
                        dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                    }
                    currDef->expr->sem();

                    long unsigned int counter = 0;
                    while (counter < dynamic_cast<Function *>(tempSE->type)->params.size()) {
                        if (dynamic_cast<Function *>(tempSE->type)->params.at(counter)->typeValue == TYPE_UNKNOWN)
                            dynamic_cast<Function *>(tempSE->type)->params.at(counter) = tempSE->params.at(counter)->type;

                        counter++;
                    }
                    if (currDef->expr->sem_getExprObj() != nullptr) {
                        if (currDef->expr->sem_getExprObj()->type->typeValue == TYPE_FUNC) {
                            // if (currDef->expr->sem_getExprObj() != tempSE) dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type->outputType;
                            // else dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type->outputType;
                        }
                        else if (currDef->expr->getType()->typeValue == TYPE_CUSTOM)
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                        else if (currDef->expr->sem_getExprObj()->type->typeValue == TYPE_ARRAY)
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type->ofType;
                        else if (currDef->expr->sem_getExprObj()->type->typeValue == TYPE_REF)
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                        else dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type;
                    }
                    else dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();

                    st.closeScope();
                }
            }
        }

        for (auto se : defsSE) se->isVisible = true;

        // st.printST();
    }

    virtual llvm::Value* compile() const override {

        // pseudoST.printST();

        for (auto currDef : defs) {
            currDef->compile();

            SymbolEntry *se = currPseudoScope->lookup(currDef->id, pseudoST.getSize());
            
            /* if def is a mutable variable/array */
            if (currDef->mut) {
                /* variable */
                if (currDef->expr == nullptr) {
                    if (se != nullptr) {                        
                        auto mutableVarMalloc = llvm::CallInst::CreateMalloc(
                            Builder.GetInsertBlock(),
                            llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                            (se->type->typeValue == TYPE_REF && se->type->ofType != nullptr && se->type->ofType->typeValue == TYPE_CUSTOM) ? se->type->getLLVMType()->getPointerElementType() : se->type->getLLVMType(),
                            llvm::ConstantExpr::getSizeOf((se->type->typeValue == TYPE_REF && se->type->ofType != nullptr && se->type->ofType->typeValue == TYPE_CUSTOM) ? se->type->getLLVMType()->getPointerElementType() : se->type->getLLVMType()),
                            nullptr,
                            nullptr,
                            se->id
                        );
                        se->Value = Builder.Insert(mutableVarMalloc);
                        return se->Value;
                    }
                    else { std::cout << "Didn't find the se\n"; std::cout.flush(); }
                }
                /* array */
                else {
                    if (se != nullptr) {

                        std::vector<llvm::Value *> dims;
                        dims.push_back(currDef->expr->compile());

                        /* size of array is at least one */
                        int dimNum = 1;

                        /* size of ith dimension saved in struct */
                        CommaExprGen *ceg = currDef->commaExprGen;
                        while (ceg != nullptr) {
                            dims.push_back(ceg->compile());
                            dimNum++;
                            ceg = ceg->getNext();
                        }

                        /* calculate total size of array */
                        llvm::Value *mulSize = dims.at(0);

                        for (long unsigned int i = 1; i < dims.size(); i++) {
                            mulSize = Builder.CreateMul(mulSize, dims.at(i));
                        }

                        
                        /* bind to se the type (so as it can be used in dim etc) */
                        se->LLVMType = se->type->getLLVMType()->getPointerElementType();

                        /* allocate to this array that will be defined a struct type */
                        // se->Value = Builder.CreateAlloca(se->LLVMType, nullptr, se->id);
                        auto arrayMalloc = llvm::CallInst::CreateMalloc(
                            Builder.GetInsertBlock(),
                            llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                            se->LLVMType,
                            llvm::ConstantExpr::getSizeOf(se->LLVMType),
                            nullptr,
                            nullptr,
                            se->id
                        );
                        se->Value = Builder.Insert(arrayMalloc);

                        auto arr = llvm::CallInst::CreateMalloc(
                            Builder.GetInsertBlock(),
                            llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                            se->type->ofType->getLLVMType(),
                            llvm::ConstantExpr::getSizeOf(se->type->ofType->getLLVMType()),
                            mulSize,
                            nullptr,
                            ""
                        );

                        Builder.Insert(arr);

                        /* append 'metadata' of the array variable { ptr_to_arr, dimsNum, dim1, dim2, ..., dimn } */
                        llvm::Value *arrayPtr = Builder.CreateGEP(se->LLVMType, se->Value, std::vector<llvm::Value *>{ c32(0), c32(0) }, "arrayPtr");
                        Builder.CreateStore(arr, arrayPtr);
                        llvm::Value *arrayDims = Builder.CreateGEP(se->LLVMType, se->Value, std::vector<llvm::Value *>{ c32(0), c32(1) }, "arrayDims");
                        Builder.CreateStore(c32(dimNum), arrayDims);
                        for (long unsigned int i = 0; i < dims.size(); i++) {
                            llvm::Value *dim = Builder.CreateGEP(se->LLVMType, se->Value, std::vector<llvm::Value *>{ c32(0), c32(i + 2) }, "dim_" + std::to_string(i));
                            Builder.CreateStore(dims.at(i), dim);
                        }
                    }

                    return nullptr;
                }
            }
            else {
                se->isVisible = false;
                /* if def is a non mutable variable - constant */
                if (currDef->parGen == nullptr) {
                    // if (se != nullptr) se->Value = (llvm::AllocaInst *)currDef->expr->compile();
                    if (se != nullptr) se->Value = currDef->expr->compile();
                    /* left for debugging */
                    else std::cout << "Symbol Entry was not found." << std::endl;
                }
                /* if def is a function */
                else {
                    if (se != nullptr) {
                        std::vector<llvm::Type *> args;

                        currPseudoScope = currPseudoScope->getNext();
                        currDef->parGen->compile();
                        
                        for (auto p : se->params) args.push_back(p->LLVMType);

                        llvm::FunctionType *fType = llvm::FunctionType::get(se->type->outputType->getLLVMType(), args, false);
                        se->Function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, se->id, TheModule.get());

                        llvm::BasicBlock *Parent = Builder.GetInsertBlock();
                        llvm::BasicBlock *FuncBB = llvm::BasicBlock::Create(TheContext, "entry", se->Function);
                        Builder.SetInsertPoint(FuncBB);
                        se->LLVMType = fType;
                        
                        
                        unsigned index = 0;

                        for (auto &Arg : se->Function->args()) Arg.setName(se->params.at(index++)->id);

                        index = 0;
                        for (auto &Arg : se->Function->args()) se->params.at(index++)->Value = &Arg;

                        llvm::Value *returnExpr = currDef->expr->compile();
                        // Builder.CreateRetVoid();
                        Builder.CreateRet(returnExpr);
                        currPseudoScope = currPseudoScope->getPrev();
                        Builder.SetInsertPoint(Parent);
                        
                    }
                    else std::cout << "Symbol Entry was not found." << std::endl;
                }
            }
            se->isVisible = true;
        }

        return nullptr;

    }

    Def *def;
    DefGen *defGen;
    std::vector<Def *> defs;
    bool rec;
};

class LetIn : public Expr, public Block {
public:
    LetIn(Let* l, Expr *e): let(l), expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "LetIn("; let->printOn(out); out <<", "; expr->printOn(out); out << ")";
    }

    virtual SymbolEntry *sem_getExprObj() override { return LetInSE; }

    virtual void sem() override {
        st.openScope();
        let->sem();
        for (auto se : st.lookup(let->def->id)->params) {
            se->isVisible = false;
        }
        expr->sem();
        LetInSE = expr->sem_getExprObj();
        this->type = expr->getType();
        st.closeScope();
    }

    virtual llvm::Value* compile() const override {
        currPseudoScope = currPseudoScope->getNext();
        let->compile();
        llvm::Value *rv = expr->compile();
        // Builder.CreateStore(rv, lv);
        currPseudoScope = currPseudoScope->getPrev();
        return rv;
    }

private:
    Let *let;
    Expr *expr;
    SymbolEntry *LetInSE;
};

class Delete : public Expr {
public:
    Delete(Expr *e): expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "Delete("; expr->printOn(out); out << ")";
    }

    virtual void sem() override {
        expr->sem();
        if (expr->getType()->typeValue != TYPE_REF) {
            /* Print Error - type mismatch */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Reference(new Unknown()));
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = new Unit();
    }

    virtual llvm::Value* compile() const override {
        return 0;
    }

private:
    Expr *expr;
};

class New : public Expr {
public:
    New(CustomType *t) { this->type = t; }

    virtual void printOn(std::ostream &out) const override {
        out << "New("; type->printOn(out); out << ")";
    }

    virtual void sem() override { this->type = new Reference(type); }

    virtual llvm::Value* compile() const override {
        return 0;
    }

};

class ArrayItem : public Expr {
public:
    ArrayItem(std::string id, Expr *e, CommaExprGen *ceg): id(id), expr(e), commaExprGen(ceg) {}

    virtual void printOn(std::ostream &out) const override {

        if (commaExprGen == nullptr) {
            out << "ArrayItem("<< id << "["; expr->printOn(out); out << "])";
        }
        else {
            out <<"ArrayItem("<< id << "["; expr->printOn(out); out <<", "; commaExprGen->printOn(out); out << "])";
        }

    }

    std::string getName() override { return id; }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(id); }

    virtual void sem() override {

        std::cout <<"In ArrayItem for " <<id <<std::endl; std::cout.flush();

        SymbolEntry *tempEntry = st.lookup(ENTRY_VARIABLE, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_PARAMETER, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_TEMP, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_CONSTANT, id);
        if (tempEntry == nullptr) {
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new FirstOccurence(id);
            err->printError();

            tempEntry = new SymbolEntry(id, new Unknown());
            /* Now Unknown is recognized as None */
            tempEntry->type->size = 0;
            this->type = tempEntry->type;
            tempEntry->entryType = ENTRY_TEMP;
            st.insert(id, tempEntry);
        }

        if (tempEntry->type->typeValue == TYPE_ARRAY && dynamic_cast<Array *>(tempEntry->type)->isInferred) dynamic_cast<Array *>(tempEntry->type)->isInferred = false;

        /* type inference */
        if (tempEntry->entryType != ENTRY_TEMP && tempEntry->type->typeValue == TYPE_UNKNOWN) {
            /* get dimensions by iterating commaExprGen "list" */
            int dims = 1;
            CommaExprGen *tempExpr = commaExprGen;
            while (tempExpr != nullptr) {
                dims++;
                tempExpr = tempExpr->getNext();
            }
            tempEntry->type = new Array(new Unknown(), dims);
        }

        expr->sem();
        if (expr->getType()->typeValue == TYPE_UNKNOWN) {
            expr->setType(new Integer());
            SymbolEntry *se = expr->sem_getExprObj();
            CustomType *iterator = se->type;
            if (se->type->typeValue == TYPE_REF) {
                while (iterator->ofType != nullptr) iterator = iterator->ofType;

                // Destroy the object but leave the space allocated.
                iterator->~CustomType();

                // Create a new object in the same space.
                iterator = new (iterator) Integer();
            }
            else {
                // Destroy the object but leave the space allocated.
                se->type->~CustomType();
                // Create a new object in the same space.
                se->type = new (se->type) Integer();
            }
        }
        if (expr->getType()->typeValue != TYPE_INT) {
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new TypeMismatch(new Integer(), expr->getType());
            err->printError();
        }

        /* set this type to type array */
        if (tempEntry->type->typeValue == TYPE_ARRAY) {
            if (tempEntry->type->ofType->typeValue == TYPE_UNKNOWN) { /* warning - polymorphic */ }
            this->type = tempEntry->type;
        }
        if (commaExprGen != nullptr) commaExprGen->sem();

        /* get dimensions by iterating commaExprGen "list" */
        int dimensions = 1;
        CommaExprGen *tempExpr = commaExprGen;
        while (tempExpr != nullptr) {
            dimensions++;
            tempExpr = tempExpr->getNext();
        }
        if (tempEntry->type->typeValue != TYPE_ARRAY) {
            /*  Print Error - type mismatch
                type mismatch in expression,
                b should be an array of 2 dimensions,
                impossible to unify @2 ref with array [*, *] of @5
            */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new ArrayTypeMismatch(dimensions, new Array(new Unknown(), dimensions), tempEntry->type);
            err->printError();
        }
        else {
            if (dimensions == tempEntry->type->size) { /* all ok */ }
            else {
                /*  Print Error
                    type mismatch in expression,
                    a should be an array of 1 dimensions,
                    impossible to unify int with @3
                */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new ArrayDimensions(new Array(new Unknown(), dimensions), dynamic_cast<Array *>(tempEntry->type));
                err->printError();
            }
        }
        if (tempEntry->type->typeValue != TYPE_UNKNOWN) this->type = tempEntry->type;
        else this->type = new Array(new Unknown(), dimensions);
    }

    virtual llvm::Value* compile() const override {
        SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
        if (se != nullptr) {
            llvm::Value *accessEl;
            std::vector<llvm::Value *> dims;
            llvm::Value *mulTemp = c32(1);

            dims.push_back(expr->compile());
            CommaExprGen *ceg = commaExprGen;
            while (ceg != nullptr) {
                dims.push_back(ceg->compile());
                ceg = ceg->getNext();
            }

            for (long unsigned int i = dims.size(); i > 0; i--) {
                if (i != dims.size()) {
                    mulTemp = Builder.CreateMul(
                        mulTemp,
                        Builder.CreateLoad(
                            Builder.CreateGEP(
                               se->LLVMType->getPointerElementType(),
                               se->Value,
                               std::vector<llvm::Value *> {c32(0), c32(i + 2)}
                            )
                        ));
                    accessEl = Builder.CreateAdd(accessEl, Builder.CreateMul(mulTemp, dims.at(i - 1)));
                }
                else {
                    accessEl = dims.at(i - 1);
                }
            }
            
            /* check access_dim.at(i) with decl_dim.at(i), if all acccess_dims are less than decl_dims all good else problem */
            llvm::Value *isCorrect = c1(true);
            llvm::Value *isGT;
            for (long unsigned int i = 0; i < dims.size(); i++) {
                isGT = Builder.CreateICmpSLT(dims.at(i), Builder.CreateLoad(Builder.CreateGEP(se->LLVMType->getPointerElementType(), se->Value, {c32(0), c32(i + 2)})));
                isCorrect = Builder.CreateAnd(isGT, isCorrect);
            }

            llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
            
            /* create ir for branch */
            llvm::BasicBlock *RuntimeExceptionBB = llvm::BasicBlock::Create(TheContext);
            llvm::BasicBlock *ContinueBB = llvm::BasicBlock::Create(TheContext);
            Builder.CreateCondBr(isCorrect, ContinueBB, RuntimeExceptionBB);     

            /* in case that access element is out of bounds */
            TheFunction->getBasicBlockList().push_back(RuntimeExceptionBB);
            Builder.SetInsertPoint(RuntimeExceptionBB);
            Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Index out of Bounds\n")) });
            Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
            Builder.CreateBr(ContinueBB);

            /* in case that all good */
            TheFunction->getBasicBlockList().push_back(ContinueBB);
            Builder.SetInsertPoint(ContinueBB);
            llvm::Value *arrPtr = Builder.CreateGEP(se->LLVMType->getPointerElementType(), se->Value, std::vector<llvm::Value *> {c32(0), c32(0)});
            arrPtr = Builder.CreateLoad(arrPtr);
            return Builder.CreateGEP(arrPtr, accessEl);
        }

        return nullptr;

    }

protected:
    std::string id;
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class Dim : public Expr {
public:
    Dim(std::string id): id(id) { intconst = 1; }
    Dim(std::string id, int ic): id(id), intconst(ic) {}

    virtual void printOn(std::ostream &out) const override { out << "Dim(" << id << ", " << intconst << ")"; }

    virtual void sem() override {
        SymbolEntry *tempEntry = st.lookup(ENTRY_VARIABLE, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_PARAMETER, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_TEMP, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_CONSTANT, id);
        if (tempEntry == nullptr) {
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new FirstOccurence(id);
            err->printError();

            tempEntry = new SymbolEntry(id, new Unknown());
            /* Now Unknown is recognized as None */
            tempEntry->type->size = 0;
            this->type = tempEntry->type;
            tempEntry->entryType = ENTRY_TEMP;
            st.insert(id, tempEntry);
        }
        this->type = new Integer();

        /* type inference */
        if (tempEntry->entryType != ENTRY_TEMP && tempEntry->type->typeValue == TYPE_UNKNOWN) {
            tempEntry->type = new Array(new Unknown(), intconst);
            dynamic_cast<Array *>(tempEntry->type)->isInferred = true;
        }

        /* type correction/inference (for Reference(Unknown)) */
        if (tempEntry->entryType != ENTRY_TEMP && tempEntry->type->typeValue == TYPE_REF && tempEntry->type->ofType->typeValue == TYPE_UNKNOWN) {
            tempEntry->type = new Array(new Unknown(), intconst);
            dynamic_cast<Array *>(tempEntry->type)->isInferred = true;
        }

        if (tempEntry->type->typeValue == TYPE_ARRAY) {
            if (dynamic_cast<Array *>(tempEntry->type)->isInferred && intconst > tempEntry->type->size) tempEntry->type->size = intconst;

            if (intconst > tempEntry->type->size) {
                /* Print Error - try to access dimension that not exists
                    type mismatch in expression,
                    a should be an array of at least 3 dimensions,
                    impossible to unify array [*, *] of int with array [*, *, *] of @3
                */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new ArrayDimensions(new Array(new Unknown(), intconst), dynamic_cast<Array *>(tempEntry->type));
                err->printError();
            }
        }
        else {
            /* Print Error - Impossible to unify type given (symbolentry's type) with array */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new ArrayTypeMismatch(intconst, new Array(new Unknown(), intconst), tempEntry->type);
            err->printError();
        }
    }

    virtual llvm::Value* compile() const override {
        SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
        if (se != nullptr) {
            return Builder.CreateLoad(Builder.CreateGEP(se->Value, std::vector<llvm::Value *>{ c32(0), c32(intconst + 1) }));
        }

        return nullptr;
    }

private:
    std::string id;
    int intconst;
};

class BinOp : public Expr {
public:
    BinOp(Expr *e1, const char * op, Expr *e2): expr1(e1), op(op), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {
        out << "BinOp("; expr1->printOn(out); out << ", " << op << ", "; expr2->printOn(out); out << ")";
    }

    virtual SymbolEntry *sem_getExprObj() { if (!strcmp(op, ";")) return expr2->sem_getExprObj(); else return nullptr; }

    virtual void sem() override {
        expr1->sem();
        expr2->sem();

        SymbolEntry *tempExpr1 = expr1->sem_getExprObj();
        SymbolEntry *tempExpr2 = expr2->sem_getExprObj();

        /* type inference for all binops exept ';' & ':=' */
        if (strcmp(op, ";") && strcmp(op, ":=")) {

            /* both unknown */
            if (tempExpr1 != nullptr && tempExpr2 != nullptr
             && tempExpr1->entryType != ENTRY_TEMP && tempExpr2->entryType != ENTRY_TEMP
             && expr1->getType()->typeValue == TYPE_UNKNOWN && expr2->getType()->typeValue == TYPE_UNKNOWN) {
                expr1->setType(expr2->getType());
                // might need change for Ref(Ref(Ref(...)))
                if (tempExpr1->type->typeValue == TYPE_REF && tempExpr1->type->ofType->typeValue == TYPE_UNKNOWN && tempExpr2->type->typeValue == TYPE_UNKNOWN)
                    tempExpr1->type->ofType = tempExpr2->type;
                else if (tempExpr2->type->typeValue == TYPE_REF && tempExpr2->type->ofType->typeValue == TYPE_UNKNOWN && tempExpr1->type->typeValue == TYPE_UNKNOWN)
                    tempExpr1->type = tempExpr2->type->ofType;
                else
                    tempExpr1->type = tempExpr2->type;
            }
            /* one expr unknown */
            else {
                if (tempExpr1 != nullptr && expr1->getType()->typeValue == TYPE_UNKNOWN && tempExpr1->entryType != ENTRY_TEMP) {
                    if (expr2->getType()->typeValue == TYPE_INT) {
                        expr1->getType()->~CustomType();
                        expr1->setType(new (expr1->getType()) Integer());
                    }
                    else if (expr2->getType()->typeValue == TYPE_FLOAT) {
                        expr1->getType()->~CustomType();
                        expr1->setType(new (expr1->getType()) Float());
                    }
                    else if (expr2->getType()->typeValue == TYPE_CHAR) {
                        expr1->getType()->~CustomType();
                        expr1->setType(new (expr1->getType()) Character());
                    }
                    else if (expr2->getType()->typeValue == TYPE_BOOL) {
                        expr1->getType()->~CustomType();
                        expr1->setType(new (expr1->getType()) Boolean());
                    }
                    else { /* for now nothing */ }


                    if (tempExpr1->type->typeValue != TYPE_FUNC) {
                        // might have problem
                        if (!(tempExpr1->type->typeValue == TYPE_ARRAY || tempExpr1->type->typeValue == TYPE_REF)) tempExpr1->type = expr1->getType();
                        else tempExpr1->type->ofType = expr1->getType();
                    }

                }
                if (tempExpr2 != nullptr && expr2->getType()->typeValue == TYPE_UNKNOWN && tempExpr2->entryType != ENTRY_TEMP) {
                    if (expr1->getType()->typeValue == TYPE_INT) {
                        expr2->getType()->~CustomType();
                        expr2->setType(new (expr2->getType()) Integer());
                    }
                    else if (expr1->getType()->typeValue == TYPE_FLOAT) {
                        expr2->getType()->~CustomType();
                        expr2->setType(new (expr2->getType()) Float());
                    }
                    else if (expr1->getType()->typeValue == TYPE_CHAR) {
                        expr2->getType()->~CustomType();
                        expr2->setType(new (expr2->getType()) Character());
                    }
                    else if (expr1->getType()->typeValue == TYPE_BOOL) {
                        expr2->getType()->~CustomType();
                        expr2->setType(new (expr2->getType()) Boolean());
                    }
                    else { /* for now nothing */ }

                    if (tempExpr2->type->typeValue != TYPE_FUNC) {
                        if (!(tempExpr2->type->typeValue == TYPE_ARRAY || tempExpr2->type->typeValue == TYPE_REF)) tempExpr2->type = expr2->getType();
                        else tempExpr2->type->ofType = expr2->getType();
                    }
                }
            }
        }

        // set size of expr1->type to zero (Now is None instead of Unknown)
        if (tempExpr1 != nullptr && tempExpr1->entryType == ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) expr1->getType()->size = 0;

        // set size of expr2->type to zero (Now is None instead of Unknown)
        if (tempExpr2 != nullptr && tempExpr2->entryType == ENTRY_TEMP && expr2->getType()->typeValue == TYPE_UNKNOWN) expr2->getType()->size = 0;

        if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "mod")) {
            this->type = new Integer();

            /* type inference - if both are unknown */
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                // expr1->setType(new Integer());
                expr2->getType()->~CustomType();
                expr2->setType(new (expr2->getType()) Integer());
            }
            if (expr1->getType()->typeValue == TYPE_INT && expr2->getType()->typeValue == TYPE_INT && expr1->getType()->ofType == nullptr && expr2->getType()->ofType == nullptr) {}
            else {
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Integer());

                Error *err;
                if (expr1->getType()->typeValue != TYPE_INT || expr1->getType()->ofType != nullptr) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr1->YYLTYPE.first_line - 1 << ", Characters " << expr1->YYLTYPE.first_column + 1 << " - " << expr1->YYLTYPE.last_column << std::endl;
                    err = new Expectation(expectedType, expr1->getType());
                    err->printError();
                }
                if (expr2->getType()->typeValue != TYPE_INT || expr2->getType()->ofType != nullptr) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column - 1 << " - " << expr2->YYLTYPE.last_column + 1 << std::endl;
                    err = new Expectation(expectedType, expr2->getType());
                    err->printError();
                }
            }
        }
        else if (!strcmp(op, "+.") || !strcmp(op, "-.") || !strcmp(op, "*.") || !strcmp(op, "/.") || !strcmp(op, "**")) {
            this->type = new Float();

            /* type inference - if both are unknown */
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                // expr1->setType(new Float());
                expr2->getType()->~CustomType();
                expr2->setType(new (expr2->getType()) Float());
            }
            if (expr1->getType()->typeValue == TYPE_FLOAT && expr2->getType()->typeValue == TYPE_FLOAT) {}
            else {
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Float());
                Error *err;
                if (expr1->getType()->typeValue != TYPE_FLOAT) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr1->YYLTYPE.first_line << ", Characters " << expr1->YYLTYPE.first_column << " - " << expr1->YYLTYPE.last_column << std::endl;
                    err = new Expectation(expectedType, expr1->getType());
                    err->printError();
                }
                if (expr2->getType()->typeValue != TYPE_FLOAT) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    err = new Expectation(expectedType, expr2->getType());
                    err->printError();
                }
            }
        }
        else if (!strcmp(op, "=") || !strcmp(op, "<>")) {
            /* the result will always be boolean */
            this->type = new Boolean();

            if(expr1->getType()->typeValue == TYPE_ID) expr1->setType(expr1->getType()->params.front());
            if(expr2->getType()->typeValue == TYPE_ID) expr2->setType(expr2->getType()->params.front());

            if (expr1->getType()->typeValue == expr2->getType()->typeValue && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {}
            else {
                /* Print Error */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, "==") || !strcmp(op, "!=")) {
            this->type = new Boolean();
            /* type check */
            if (expr1->getType()->typeValue == expr2->getType()->typeValue
             && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {}
            else {
                /* Print Error */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, ">=") || !strcmp(op, "<=")) {
            this->type = new Boolean();
            if (expr1->getType()->typeValue == expr2->getType()->typeValue
             && (expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT
             || expr1->getType()->typeValue == TYPE_CHAR || expr1->getType()->typeValue == TYPE_UNKNOWN)) {}
            else {
                /* Print Error */
                std::vector<CustomType *> expectedTypes;
                expectedTypes.push_back(new Integer());
                expectedTypes.push_back(new Float());
                expectedTypes.push_back(new Character());
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err;
                if (!(expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT || expr1->getType()->typeValue == TYPE_CHAR)) err = new Expectation(expectedTypes, expr1->getType());
                if (!(expr2->getType()->typeValue == TYPE_INT || expr2->getType()->typeValue == TYPE_FLOAT || expr2->getType()->typeValue == TYPE_CHAR)) err = new Expectation(expectedTypes, expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, "&&") || !strcmp(op, "||")) {
            this->type = new Boolean();
            if (expr1->getType()->typeValue == expr2->getType()->typeValue && expr1->getType()->typeValue == TYPE_BOOL) {}
            else {
                /* Print Error */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, ";")) {
            this->type = expr2->getType();
        }
        else if (!strcmp(op, ":=")) {

            this->type = new Unit();
            bool recursiveRefError = false;
            
            /* type inference */
            if(expr2->getType()->typeValue == TYPE_ID) expr2->setType(expr2->getType()->params.front());

            SymbolEntry *tempEntry = expr1->sem_getExprObj();
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_ARRAY && expr1->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                // if (expr2->getType()->typeValue != TYPE_ID) {
                //     expr1->getType()->ofType = expr2->getType();
                //     tempEntry->type->ofType = expr2->getType();
                // }
                // else {
                //     expr1->getType()->ofType = tempExpr2->params.front()->type;
                //     tempEntry->type->ofType = tempExpr2->params.front()->type;
                // }
                expr1->getType()->ofType = expr2->getType();
                tempEntry->type->ofType = expr2->getType();
            }
            else {
                if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                    if (tempExpr2 != nullptr) {
                        // expr1->setType(new Reference(expr2->getType()));
                        std::pair <CustomType *, int> pairExpr1, pairExpr2;
                        pairExpr1 = expr1->getRefFinalType(tempExpr1->type);
                        if(expr2->getType()->typeValue == TYPE_CUSTOM) {
                            if (tempExpr2->params.front()->type->typeValue != TYPE_FUNC)
                                pairExpr2 = expr2->getRefFinalType(tempExpr2->params.front()->type);
                            else pairExpr2 = expr2->getFnFinalType(tempExpr2->type);
                        }
                        else {
                            if (tempExpr2->type->typeValue != TYPE_FUNC)
                            pairExpr2 = expr2->getRefFinalType(tempExpr2->type);
                            else pairExpr2 = expr2->getFnFinalType(tempExpr2->type);
                        }
                        if (pairExpr1.first != pairExpr2.first) {
                            /* change SumbolEntry type */
                            CustomType *tempCT = tempExpr1->type;
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(pairExpr2.first);
                            /* Change expr type */
                            // std::cout <<"About to set tempexpr1 to ref of "; pairExpr2.first->printOn(std::cout); std::cout <<std::endl; std::cout.flush();
                            // std::cout <<"About to set expr1 to ref of "; expr2->getType()->printOn(std::cout); std::cout <<std::endl; std::cout.flush();
                            expr1->setType(new Reference(expr2->getType()));
                            
                        }
                        else {
                            recursiveRefError = true;
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempExpr1->type, tempExpr2->params.front()->type);
                            err->printError();
                        }
                    }
                    else {
                        CustomType *tempCT = tempExpr1->type;
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(expr2->getType());
                        /* Change expr type */
                        expr1->setType(new Reference(expr2->getType()));
                    }
                }
            }

            /* synchronize expr with tempExpr1 */
            if (expr1->getType()->typeValue == TYPE_REF && tempExpr1->type->typeValue == TYPE_UNKNOWN) {
                tempExpr1->type = expr1->getType();
            }
            else if (expr1->getType()->typeValue != TYPE_REF && tempExpr1->type->typeValue == TYPE_REF) {
                expr1->setType(tempExpr1->type);
            }

            if (tempEntry != nullptr && tempEntry->entryType != ENTRY_TEMP) {
                // if expr1 = Ref(Unknown) then replace Unknown with expr2 type
                if (expr1->getType()->typeValue == TYPE_REF && expr1->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                    /* edge case for when second operand points to first */
                    if (expr2->getType()->typeValue == TYPE_REF
                    && getRefFinalType(expr1->getType()).first == getRefFinalType(expr2->getType()).first) {
                        /* Print Error - type mismatch */
                        if (expr1->YYLTYPE.first_line == expr2->YYLTYPE.first_line) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << expr1->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                        }
                        else {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Lines " << expr1->YYLTYPE.first_line << " - " << expr2->YYLTYPE.last_line << std::endl;
                        }

                        Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                        err->printError();
                    }
                    else {
                        expr1->getType()->ofType = expr2->getType();
                    }
                }
                else if (expr1->getType()->typeValue == TYPE_ARRAY) {
                    if (expr1->getType()->ofType->typeValue != expr2->getType()->typeValue) {
                        /* Print Error - type mismatch */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                        err->printError();
                    }
                }
                // expr1 already has a ref type so need to compare type with expr2 type
                else {

                    std::pair <CustomType *, int> pairExpr1, pairExpr2;

                    if (expr1->getType()->typeValue == TYPE_REF) pairExpr1 = getRefFinalType(expr1->getType()->ofType);
                    // if (expr1->getType()->typeValue == TYPE_REF) pairExpr1 = getRefFinalType(tempExpr1->type->ofType);
                    if (expr2->getType()->typeValue == TYPE_REF) pairExpr2 = getRefFinalType(expr2->getType());

                    if (!recursiveRefError) {
                        if (expr1->getType()->ofType->typeValue == expr2->getType()->typeValue) {

                            if (expr2->getType()->typeValue == TYPE_REF) {
                                if (pairExpr1.first->typeValue == pairExpr2.first->typeValue && pairExpr1.second == pairExpr2.second) {}
                                else {
                                    /* Print Error */
                                    semError = true;
                                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                                    Error *err = new TypeMismatch(pairExpr1.first, pairExpr2.first);
                                    err->printError();
                                }
                            }
                            else {}

                        }
                        else {
                            /* Print Error - type mismatch */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                            err->printError();
                        }
                    }
                }
            }
            else {
                /* Print Error */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;


                // if needed, can check expr1 name as follows:
                // std::string str = expr1->getName();
                // if (str.length() == 0) {...}

                CustomType *typeOfExpr2 = expr2->getType();
                CustomType *tempRefType = new Reference(new Unknown());
                tempRefType->ofType = nullptr;
                typeOfExpr2->ofType = tempRefType;

                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }

    }

    llvm::Value *constrsEqCheck(llvm::Value *constr1, llvm::Value *constr2, llvm::Value *isMatch = c1(true)) {

        llvm::Value *constr1_tag = Builder.CreateLoad(Builder.CreateGEP(constr1, { c32(0), c32(0) }));
        llvm::Value *constr2_tag = Builder.CreateLoad(Builder.CreateGEP(constr2, { c32(0), c32(0) }));   

        if (constr1_tag == nullptr && constr2_tag == nullptr) return isMatch;
        else if ((constr1_tag == nullptr && constr2_tag != nullptr) 
              || (constr1_tag != nullptr && constr2_tag == nullptr)) return c1(false); 
        else {
            isMatch = Builder.CreateAnd(isMatch, Builder.CreateICmpEQ(constr1_tag, constr2_tag));
            constrsEqCheck(constr1_tag, constr2_tag, isMatch);
        }
    }

    virtual llvm::Value* compile() const override {
        llvm::Value *lv = expr1->compile();
        llvm::Value *rv = expr2->compile();

        if (lv != nullptr && lv->getType()->isPointerTy() && strcmp(op, ":=") && getRefFinalType(expr1->getType()).first->typeValue != TYPE_CUSTOM) lv = Builder.CreateLoad(lv);
        if (rv != nullptr && rv->getType()->isPointerTy() && strcmp(op, ";") && getRefFinalType(expr2->getType()).first->typeValue != TYPE_CUSTOM) rv = Builder.CreateLoad(rv);

        if (!strcmp(op, "+")) return Builder.CreateAdd(lv, rv);
        else if (!strcmp(op, "-")) return Builder.CreateSub(lv, rv);
        else if (!strcmp(op, "*")) return Builder.CreateMul(lv, rv);
        else if (!strcmp(op, "/")) {
            llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
            
            /* check if rhs is eq to zero */
            llvm::Value *isZero = Builder.CreateICmpEQ(rv, c32(0));

            /* create ir for branch */
            llvm::BasicBlock *RuntimeExceptionBB = llvm::BasicBlock::Create(TheContext);
            llvm::BasicBlock *ContinueBB = llvm::BasicBlock::Create(TheContext);
            Builder.CreateCondBr(isZero, RuntimeExceptionBB, ContinueBB);

            /* in case that it is, then Runtime Exception should be raised */
            TheFunction->getBasicBlockList().push_back(RuntimeExceptionBB);
            Builder.SetInsertPoint(RuntimeExceptionBB);
            Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Division with Zero\n")) });
            Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
            Builder.CreateBr(ContinueBB);

            /* else should continue normally */
            TheFunction->getBasicBlockList().push_back(ContinueBB);
            Builder.SetInsertPoint(ContinueBB);
            return Builder.CreateSDiv(lv, rv);
        }
        else if (!strcmp(op, "mod")) return Builder.CreateSRem(lv, rv);
        else if (!strcmp(op, "+.")) return Builder.CreateFAdd(lv, rv);
        else if (!strcmp(op, "-.")) return Builder.CreateFSub(lv, rv);
        else if (!strcmp(op, "*.")) return Builder.CreateFMul(lv, rv);
        else if (!strcmp(op, "/.")) {
            llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

            /* check if rhs is eq to zero */
            llvm::Value *isZero = Builder.CreateFCmpOEQ(rv, fp(0));

            /* create ir for branch */
            llvm::BasicBlock *RuntimeExceptionBB = llvm::BasicBlock::Create(TheContext);
            llvm::BasicBlock *ContinueBB = llvm::BasicBlock::Create(TheContext);
            Builder.CreateCondBr(isZero, RuntimeExceptionBB, ContinueBB);

            /* in case that it is, then Runtime Exception should be raised */
            TheFunction->getBasicBlockList().push_back(RuntimeExceptionBB);
            Builder.SetInsertPoint(RuntimeExceptionBB);
            Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Division with Zero\n")) });
            Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
            Builder.CreateBr(ContinueBB);

            /* else should continue normally */
            TheFunction->getBasicBlockList().push_back(ContinueBB);
            Builder.SetInsertPoint(ContinueBB);
            return Builder.CreateFDiv(lv, rv);
        }
        else if (!strcmp(op, "**")) return Builder.CreateBinaryIntrinsic(llvm::Intrinsic::pow, lv, rv, nullptr, "float.powtmp");
        /*  implementation needed for:
            TYPE_CUSTOM
            TYPE_ID
            in operands =, ==, <>, !=
         */
        else if (!strcmp(op, "=")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_CUSTOM:
                    break;
                case TYPE_UNIT:
                    return c1(true);
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_OEQ, lv, rv);
                case TYPE_BOOL:
                    return Builder.CreateNot(Builder.CreateOr(lv, rv));
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_EQ, lv, rv);
            }
        }
        else if (!strcmp(op, "<>")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_CUSTOM:
                    break;
                case TYPE_UNIT:
                    return c1(false);
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_ONE, lv, rv);
                case TYPE_BOOL:
                    return Builder.CreateOr(lv, rv);
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_NE, lv, rv);
            }
        }
        else if (!strcmp(op, "==")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_CUSTOM:
                    return Builder.CreateICmpEQ(Builder.CreatePtrDiff(lv, rv), c64(0));
                case TYPE_UNIT:
                    return c1(true);
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_OEQ, lv, rv);
                case TYPE_BOOL:
                    return Builder.CreateNot(Builder.CreateOr(lv, rv));
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_EQ, lv, rv);
            }
        }
        else if (!strcmp(op, "!=")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_CUSTOM:
                    return Builder.CreateICmpNE(Builder.CreatePtrDiff(lv, rv), c64(0));
                case TYPE_UNIT:
                    return c1(false);
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_ONE, lv, rv);
                case TYPE_BOOL:
                    return Builder.CreateOr(lv, rv);
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_NE, lv, rv);
            }
        }
        else if (!strcmp(op, "<")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_OLT, lv, rv);
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_SLT, lv, rv);
            }
        }
        else if (!strcmp(op, ">")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_OGT, lv, rv);
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_SGT, lv, rv);
            }
        }
        else if (!strcmp(op, ">=")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_OGE, lv, rv);
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_SGE, lv, rv);
            }
        }
        else if (!strcmp(op, "<=")) {
            switch (getRefFinalType(expr1->getType()).first->typeValue) {
                case TYPE_FLOAT:
                    return Builder.CreateFCmp(llvm::CmpInst::FCMP_OLE, lv, rv);
                case TYPE_CHAR:
                default:
                    return Builder.CreateICmp(llvm::CmpInst::ICMP_SLE, lv, rv);
            }
        }
        else if (!strcmp(op, "&&")) return Builder.CreateAnd(lv, rv);
        else if (!strcmp(op, "||")) return Builder.CreateOr(lv, rv);
        else if (!strcmp(op, ";")) return rv;
        else if (!strcmp(op, ":=")) {
            if (rv->getType()->isPointerTy()) rv = Builder.CreateLoad(rv);
            Builder.CreateStore(rv, lv);
            return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
        }

        return nullptr;
    }

private:
    Expr *expr1;
    const char * op;
    Expr *expr2;
};

class UnOp : public Expr {
public:
    UnOp(const char * op, Expr *e): op(op), expr(e)  {}

    virtual void printOn(std::ostream &out) const override {
        out << "UnOp(" << op <<", "; expr->printOn(out); out <<")";
    }

    virtual SymbolEntry *sem_getExprObj() override { return expr->sem_getExprObj(); }

    std::string getName() override { return expr->getName(); }

    virtual void sem() override {
        expr->sem();
        if (!strcmp(op, "!")) {
            /* If expr is Ref(type), make Dereference (convert Ref(type) to type) */
            /* or if expr is Array(type), make this type eq to array oftype */
            if (expr->getType()->typeValue == TYPE_REF || expr->getType()->typeValue == TYPE_ARRAY) {
                this->type = expr->getType()->ofType;
                // if (this->type->typeValue == TYPE_ID) this->type = this->type->params.front();
            }
            else {
                /* type inference */
                if (expr->getType()->typeValue == TYPE_UNKNOWN) {

                    if (expr->sem_getExprObj() != nullptr) {
                        CustomType *tempCT = expr->sem_getExprObj()->type;

                        if (tempCT->typeValue == TYPE_UNIT) {
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(new Unit());
                        }
                        else if (tempCT->typeValue == TYPE_INT) {
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(new Integer());
                        }
                        else if (tempCT->typeValue == TYPE_CHAR) {
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(new Character());
                        }
                        else if (tempCT->typeValue == TYPE_BOOL) {
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(new Boolean());
                        }
                        else if (tempCT->typeValue == TYPE_FLOAT) {
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(new Float());
                        }
                        else if (tempCT->typeValue == TYPE_REF) {
                            std::pair <CustomType *, int> pairTempCT = getRefFinalType(tempCT);
                            CustomType *newFinalType;

                            if (pairTempCT.first->typeValue == TYPE_UNIT) newFinalType = new Unit();
                            else if (pairTempCT.first->typeValue == TYPE_INT) newFinalType = new Integer();
                            else if (pairTempCT.first->typeValue == TYPE_CHAR) newFinalType = new Character();
                            else if (pairTempCT.first->typeValue == TYPE_BOOL) newFinalType = new Boolean();
                            else if (pairTempCT.first->typeValue == TYPE_FLOAT) newFinalType = new Float();
                            // else if (pairTempCT.first->typeValue == TYPE_ID) newFinalType = new Integer();
                            else if (pairTempCT.first->typeValue == TYPE_UNKNOWN) newFinalType = new Unknown();
                            else if (pairTempCT.first->typeValue == TYPE_CUSTOM) newFinalType = new CustomType();

                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(newFinalType);

                            pairTempCT.second--;
                            while (pairTempCT.second != 0) {
                                tempCT = expr->sem_getExprObj()->type;
                                expr->sem_getExprObj()->type = new Reference(tempCT);
                                pairTempCT.second--;
                            }
                        }
                        else if (tempCT->typeValue == TYPE_UNKNOWN) {
                            tempCT->~CustomType();
                            tempCT = new (tempCT) Reference(new Unknown());
                        }
                        else {
                            expr->sem_getExprObj()->type = new Reference(expr->sem_getExprObj()->type);
                        }

                        // expr->sem_getExprObj()->type = new Reference(expr->sem_getExprObj()->type);
                        expr->setType(new Reference(getRefFinalType(expr->sem_getExprObj()->type).first));
                    }
                    else expr->setType(new Reference(expr->getType()));
                    this->type = expr->getType()->ofType;
                }
                /* If expr is not Ref(type) - reached the end of References - make an Invalid type Type(Ref(nullptr)) */
                else {
                    CustomType *tempRef = new Reference(new Unknown());
                    tempRef->ofType = nullptr;

                    if (expr->sem_getExprObj() != nullptr) expr->sem_getExprObj()->type->ofType = tempRef;

                    // this->type->ofType = new Reference(this->type->ofType);
                    this->type = expr->sem_getExprObj()->type;
                }
            }
        }
        else if (!strcmp(op, "+")) {
            if (expr->getType()->typeValue != TYPE_INT) {
                /* Print Error */
                semError = true;
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Integer());
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "-")) {
            if (expr->getType()->typeValue != TYPE_INT) {
                /* Print Error */
                semError = true;
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Integer());
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "+.")) {
            if (expr->getType()->typeValue != TYPE_FLOAT) {
                /* Print Error */
                semError = true;
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Float());
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "-.")) {
            if (expr->getType()->typeValue != TYPE_FLOAT) {
                /* Print Error */
                semError = true;
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Float());
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "not")) {
            if (expr->getType()->typeValue != TYPE_BOOL) {
                /* Print Error */
                semError = true;
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Boolean());
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else { /* Left for debugging */ }
    }

    virtual llvm::Value* compile() const override {
        llvm::Value *v = expr->compile();
        if (!strcmp(op, "!")) {
            CustomType *t = currPseudoScope->lookup(expr->getName(), pseudoST.getSize())->type;
            if ((t->typeValue == TYPE_ARRAY && t->ofType->typeValue == TYPE_CHAR)
            || (t->typeValue == TYPE_REF && t->ofType->typeValue == TYPE_ARRAY && t->ofType->ofType->typeValue == TYPE_CHAR)) return v;
            if (v->getType()->isPointerTy() && t->ofType != nullptr && t->ofType->typeValue != TYPE_CUSTOM) 
                return Builder.CreateLoad(v);

            return v;
        }
        else if (!strcmp(op, "+")) return v;
        else if (!strcmp(op, "-")) {
            return Builder.CreateMul(v, c32(-1));
        }
        else if (!strcmp(op, "+.")) return v;
        else if (!strcmp(op, "-.")) {
            return Builder.CreateFMul(v, fp(-1.0));
        }
        else if (!strcmp(op, "not")) return Builder.CreateNot(v);
        return nullptr;
    }

private:
    const char * op;
    Expr *expr;
};

class IntConst : public Constant, public Pattern {
public:
    IntConst(int ic, bool b = false): isPattern(b) { intConst = ic; type = new Integer(); }
    IntConst(int ic, char s, bool b = false): isPattern(b) {
        intConst = (s == '+') ? ic : -ic;
        type = new Integer();
    }

    virtual void printOn(std::ostream &out) const override {
        out << intConst;
    }

    virtual void sem() override { this->type = new Integer(); }

    virtual llvm::Value* compile() const override {
        if (isPattern) return Builder.CreateICmpEQ(c32(intConst), matchExprV);
        else return c32(intConst);
    }

private:
    int intConst;
    bool isPattern;
};

class FloatConst : public Constant, public Pattern {
public:
    FloatConst(double fc, bool b = false): isPattern(b) { floatConst = fc; type = new Float(); }
    FloatConst(double fc, const char * s, bool b = false): isPattern(b) {
        floatConst = ( strcmp(s, "+.") == 0 ) ? fc : -fc;
        type = new Float();
    }

    virtual void printOn(std::ostream &out) const override {
        out << floatConst;
    }

    virtual void sem() override { this->type = new Float(); }

    virtual llvm::Value* compile() const override {
        if (isPattern) return Builder.CreateFCmpOEQ(fp(floatConst), matchExprV);
        else return fp(floatConst);
    }

private:
    double floatConst;
    bool isPattern;
};

class CharConst : public Constant, public Pattern {
public:
    CharConst(std::string cc, bool b = false): isPattern(b) { 
        
        if (cc.at(1) == '\\' && cc.at(2) != '\'') {
            switch (cc.at(2)) {
            case 't':
                charConst = '\t';
                break;
            case 'n':
                charConst = '\n';
                break;
            case 'r':
                charConst = '\r';
                break;
            case '\\':
                charConst = '\\';
                break;
            case '\'':
                charConst = '\'';
                break;
            case '\"':
                charConst = '\"';
                break;
            default:
                charConst = '\0';
                break;
            }
        }
        else charConst = cc.at(1);

        type = new Character();
    }

    virtual void printOn(std::ostream &out) const override {
        out << charConst;
    }

    virtual void sem() override { this->type = new Character(); }

    virtual llvm::Value* compile() const override {
        if (isPattern) return Builder.CreateICmpEQ(c8(charConst), matchExprV);
        else return c8(charConst);
    }

private:
    char charConst;
    bool isPattern;
};

class StringLiteral : public Constant, public Expr {
public:
    StringLiteral(std::string sl): stringLiteral(sl) {
        stringLiteral = stringLiteral.substr(1, stringLiteral.size() - 2);

        while (stringLiteral.find("\\n") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\n"), 2, "\n");
        while (stringLiteral.find("\\t") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\t"), 2, "\t");
        while (stringLiteral.find("\\r") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\r"), 2, "\r");
        while (stringLiteral.find("\\0") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\0"), 2, "\0");
        while (stringLiteral.find("\\\\") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\\\"), 2, "\\");
        while (stringLiteral.find("\\'") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\'"), 2, "\'");
        while (stringLiteral.find("\\\"") != std::string::npos) stringLiteral.replace(stringLiteral.find("\\\""), 2, "\"");

        type = new Array(new Character(), 1);
    }

    virtual void printOn(std::ostream &out) const override {
        out << stringLiteral;
    }

    virtual void sem() override { this->type = new Array(new Character(), 1); }

    virtual llvm::Value* compile() const override {

        llvm::StructType *arrayStruct = TheModule->getTypeByName("Array_String_1");

        /* allocate to this array that will be defined a struct type */
        // llvm::Value *stringV = Builder.CreateAlloca(arrayStruct, nullptr, stringLiteral);
        auto stringVarMalloc = llvm::CallInst::CreateMalloc(
            Builder.GetInsertBlock(),
            llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
            arrayStruct,
            llvm::ConstantExpr::getSizeOf(arrayStruct),
            nullptr,
            nullptr,
            ""
        );
        llvm::Value *stringV = Builder.Insert(stringVarMalloc);


        auto arr = llvm::CallInst::CreateMalloc(
            Builder.GetInsertBlock(),
            llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
            i8,
            llvm::ConstantExpr::getSizeOf(i8),
            c32(stringLiteral.length()),
            nullptr,
            ""
        );

        Builder.Insert(arr);

        /* append 'metadata' of the array variable { ptr_to_arr, dimsNum, dim1, dim2, ..., dimn } */
        llvm::Value *arrayPtr = Builder.CreateGEP(arrayStruct, stringV, std::vector<llvm::Value *>{ c32(0), c32(0) }, "stringLiteral");
        Builder.CreateStore(arr, arrayPtr);
        llvm::Value *arrayDims = Builder.CreateGEP(arrayStruct, stringV, std::vector<llvm::Value *>{ c32(0), c32(1) }, "stringDim");
        Builder.CreateStore(c32(1), arrayDims);
        llvm::Value *dim = Builder.CreateGEP(arrayStruct, stringV, std::vector<llvm::Value *>{ c32(0), c32(2) }, "dim_0");
        Builder.CreateStore(c32(stringLiteral.length()), dim);

        /* add the string to the array */
        std::vector<llvm::Value *> args;
        args.push_back(Builder.CreateLoad(arrayPtr));
        args.push_back(Builder.CreateGlobalStringPtr(llvm::StringRef(stringLiteral)));
        Builder.CreateCall(TheStringCopy, args);

        return stringV;
    }

private:
    std::string stringLiteral;
};

class BooleanConst : public Constant, public Pattern {
public:
    BooleanConst(bool b, bool bp = false): boolean(b), isPattern(bp) { type = new Boolean(); }

    virtual void printOn(std::ostream &out) const override {
        (boolean) ? out << "true" : out << "false";
    }

    virtual void sem() override { this->type = new Boolean(); }

    virtual llvm::Value* compile() const override {
        if (isPattern) return Builder.CreateAnd(c1(boolean), matchExprV);
        else return c1(boolean);
    }

private:
    const bool boolean;
    bool isPattern;
};

class UnitConst : public Constant, public Expr {
public:
    UnitConst() { type = new Unit(); }

    virtual void printOn(std::ostream &out) const override {
        out << "unit";
    }

    virtual void sem() override { this->type = new Unit(); }

    virtual llvm::Value* compile() const override {
        return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
    }

};

class TypeGen : public AST {
public:
    TypeGen(CustomType *t, TypeGen *tg): type(t), typeGen(tg) {}

    virtual void printOn(std::ostream &out) const override {

        if (typeGen == nullptr) {
            out << "TypeGen("; type->printOn(out); out << ")";
        }
        else {
            out << "TypeGen("; typeGen->printOn(out); out << ", "; type->printOn(out); out << ")";
        }

    }

    virtual void sem() override {
        if (typeGen != nullptr) typeGen->sem();
    }

    virtual llvm::Value* compile() const override {
        return 0;
    }

    CustomType *type;
    TypeGen *typeGen;
};

class Constr : public Expr {
public:
    Constr(std::string id, TypeGen *tg): Id(id), typeGen(tg) { call = false; }
    Constr(std::string id, Expr *e, ExprGen *eg): Id(id), expr(e), exprGen(eg) { call = true; }

    virtual void printOn(std::ostream &out) const override {
        if (expr == nullptr) {
            if (typeGen == nullptr) {
                out << "Constr(" << Id << ")" ;
            }
            else {
                out << "Constr(" << Id << ", "; typeGen->printOn(out); out << ")";
            }
        }
        else {
            if (exprGen == nullptr) {
                out << "ID(" <<Id <<", "; expr->printOn(out); out << ")";
            }
            else {
                out << "ID(" <<Id <<", "; expr->printOn(out); out << ", "; exprGen->printOn(out); out << ")";
            }
        }
    }

    virtual void sem() override {
        /* Constructor (Id) is called therefore need to give this Expr class the SymbolEntry's type and type check its params */
        if (call) {
            SymbolEntry *tempEntry = st.lookup(Id);
            if (tempEntry != nullptr) {
                /* Type check first param */
                if (expr != nullptr) {
                    expr->sem();
                    /* type inference */
                    if (expr->getType()->typeValue == TYPE_UNKNOWN && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0)->typeValue != TYPE_UNKNOWN) {
                        SymbolEntry *tempParam = expr->sem_getExprObj();
                        if (tempParam->type->typeValue == TYPE_FUNC && dynamic_cast<Function*>(tempParam->type)->outputType != nullptr && dynamic_cast<Function*>(tempParam->type)->outputType->typeValue == TYPE_UNKNOWN) {
                            dynamic_cast<Function*>(tempParam->type)->outputType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                            expr->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0));
                        }
                        else if (tempParam->type->typeValue == TYPE_REF && tempParam->type->ofType->typeValue == TYPE_UNKNOWN) {
                            tempParam->type->ofType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                            expr->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0));
                        }
                        else {
                            tempParam->type = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                            expr->setType(tempParam->type);
                        }
                    }
                    /* type check */
                    if (expr->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0)->typeValue) {
                        SymbolEntry *se;
                        if (expr->getType()->typeValue == TYPE_ID) se = st.lookup(expr->getType()->name);
                        if (se != nullptr && expr->getType()->typeValue == TYPE_ID && se->type->typeValue == TYPE_ID && dynamic_cast<CustomId*>(tempEntry->type)->getParams().front()->typeValue == TYPE_CUSTOM
                        && se->params.front()->id != dynamic_cast<CustomId*>(tempEntry->type)->getParams().front()->name) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(expr->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().front());
                            err->printError();
                        }
                    }
                }
                /* Type check the rest of the params */
                if (exprGen != nullptr) {
                    exprGen->sem();
                    ExprGen* tempExprGen = exprGen;
                    int i = 1;
                    while (tempExprGen != nullptr) {
                        /* type inference */
                        if (tempExprGen->getExpr()->getType()->typeValue == TYPE_UNKNOWN && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue != TYPE_UNKNOWN) {
                            SymbolEntry *tempParam = tempExprGen->getExpr()->sem_getExprObj();
                            if (tempParam->type->typeValue == TYPE_FUNC && dynamic_cast<Function*>(tempParam->type)->outputType != nullptr && dynamic_cast<Function*>(tempParam->type)->outputType->typeValue == TYPE_UNKNOWN) {
                                dynamic_cast<Function*>(tempParam->type)->outputType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i);
                                tempExprGen->getExpr()->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                            }
                            else if (tempParam->type->typeValue == TYPE_REF && tempParam->type->ofType->typeValue == TYPE_UNKNOWN) {
                                tempParam->type->ofType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i);
                                tempExprGen->getExpr()->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                            }
                            else {
                                tempParam->type = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i);
                                tempExprGen->getExpr()->setType(tempParam->type);
                            }
                        }
                        /* type check */
                        if (tempExprGen->getExpr()->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue) {
                            /* Print Error - type mismatch on ith param */
                            /* if it's another contructor */
                            if (tempExprGen->getExpr()->getType()->typeValue == TYPE_ID && tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_ID && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue == TYPE_CUSTOM
                            && tempExprGen->getExpr()->sem_getExprObj()->params.front()->type->name != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->name) {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << tempExprGen->getExpr()->YYLTYPE.first_column << " - " << tempExprGen->getExpr()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(tempExprGen->getExpr()->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                                err->printError();
                            }
                            /* if it's a different type */
                            if ((tempExprGen->getExpr()->getType()->typeValue != TYPE_ID || tempExprGen->getExpr()->sem_getExprObj()->type->typeValue != TYPE_ID || dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue != TYPE_CUSTOM)
                            && tempExprGen->getExpr()->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue) {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << tempExprGen->getExpr()->YYLTYPE.first_line << ", Characters " << tempExprGen->getExpr()->YYLTYPE.first_column << " - " << tempExprGen->getExpr()->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(tempExprGen->getExpr()->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                                err->printError();
                            }
                        }
                        i++;
                        tempExprGen = tempExprGen->getNext();
                    }
                }
                // this->type = tempEntry->type;
                // this->type->params.push_back(tempEntry->params.front()->type);
                //CHANGE: return CustomType instead of CustomId - TYPE_ID exists only in SymbolEntries
                this->type = tempEntry->params.front()->type;
            }
            else {
                this->type = new Unknown();
                /* Print Error - Id not exist in st */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new FirstOccurence(Id);
                err->printError();
            }
        }
        else {
            CustomType *ct = new CustomId(Id);
            this->type = ct;
            if (typeGen != nullptr) {
                typeGen->sem();

                TypeGen *tempTypeGen = typeGen;
                while (tempTypeGen != nullptr) {
                    if (typeGen->type->typeValue == TYPE_ID && !st.lookup(typeGen->type->name)) {
                        /* Print Error */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new Error(typeGen->type->name + " doesn't exist in ST");
                        err->printMessage();
                        exit(1);
                    }
                    dynamic_cast<CustomId*>(ct)->pushToParams(tempTypeGen->type);
                    tempTypeGen = tempTypeGen->typeGen;
                }
            }

            st.insert(Id, ct, ENTRY_CONSTRUCTOR);
        }
    }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(Id); }

    std::string getName() { return Id; }

    void defineConstr(SymbolEntry *se) const {
        std::string constrName = se->params.front()->id + "_" + se->id;

        /* create constr */
        std::vector<llvm::Type *> members;

        /* tag */
        members.push_back(i32);

        /* append all necessary fields in constructor Struct */
        for (auto p : dynamic_cast<CustomId *>(se->type)->getParams()) {
            members.push_back(p->getLLVMType());
        }

        /* create the constr */
        llvm::StructType *constrStruct = llvm::StructType::create(TheContext, constrName);
        constrStruct->setBody(members);

        se->LLVMType = constrStruct;
    }

    virtual llvm::Value* compile() const override {

        SymbolEntry *se = currPseudoScope->lookup(Id, pseudoST.getSize());
        if (!call) {
            if (se != nullptr) defineConstr(se);
        }
        else {
            auto structMalloc = llvm::CallInst::CreateMalloc(
                Builder.GetInsertBlock(),
                llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                se->LLVMType,
                llvm::ConstantExpr::getSizeOf(se->LLVMType),
                nullptr,
                nullptr,
                ""
            );

            llvm::Value *v = Builder.Insert(structMalloc);

            llvm::Value *tag = Builder.CreateGEP(se->LLVMType, v, std::vector<llvm::Value *>{ c32(0), c32(0) }, "tag");
            std::vector<SymbolEntry *> udtSE = se->params.front()->params;
            int index;
            for (long unsigned int i = 0; i < udtSE.size(); i++) {
                if (se == udtSE.at(i)) index = i;
            }
            Builder.CreateStore(c32(index), tag);
            
            if (expr != nullptr) {
                llvm::Value *temp = Builder.CreateGEP(se->LLVMType, v, std::vector<llvm::Value *>{ c32(0), c32(1) }, "temp");
                Builder.CreateStore(expr->compile(), temp);
            }
            if (exprGen != nullptr) {
                index = 2;
                ExprGen *tempExprGen = exprGen;
                llvm::Value *temp;
                while (tempExprGen != nullptr) {
                    temp = Builder.CreateGEP(se->LLVMType, v, std::vector<llvm::Value *>{ c32(0), c32(index++) }, "temp");
                    Builder.CreateStore(tempExprGen->compile(), temp);
                    tempExprGen = tempExprGen->getNext();
                }
            }

            /* in case the expr is a constructor need bitcast to convert v to base type */
            SymbolEntry *exprSE = currPseudoScope->lookup(Id, pseudoST.getSize());
            return Builder.CreatePointerCast(structMalloc, exprSE->params.front()->LLVMType->getPointerTo());
            
        }

        return nullptr;
    }

private:
    std::string Id;
    TypeGen *typeGen;
    Expr *expr;
    ExprGen *exprGen;
    bool call;
};

class BarConstrGen : public AST {
public:
    BarConstrGen(Constr *c, BarConstrGen *bcg): constr(c), barConstrGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        if (barConstrGen == nullptr) {
            out << "BarConstrGen("; constr->printOn(out); out << ")";
        }
        else {
            out << "BarConstrGen("; constr->printOn(out); out << ", "; barConstrGen->printOn(out); out << ")";
        }

    }

    Constr *getConstr() { return constr; }

    BarConstrGen *getNext() { return barConstrGen; }

    virtual llvm::Value* compile() const override {
        constr->compile();
        if (barConstrGen != nullptr) barConstrGen->compile();

        return nullptr;
    }

private:
    Constr *constr;
    BarConstrGen *barConstrGen;
};

class Tdef : public AST {
public:
    Tdef(std::string id, Constr *c, BarConstrGen *bcg): id(id), constr(c), barConstrGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        if (barConstrGen == nullptr) {
            out << "Tdef("<< id <<", "; constr->printOn(out); out << ")";
        }
        else {
            out << "Tdef("<< id <<", "; constr->printOn(out); out << ", "; barConstrGen->printOn(out); out << ")";
        }

    }

    std::string getName() { return id; }

    BarConstrGen *getBarConstrGen() { return barConstrGen; }

    virtual void sem() override {
        SymbolEntry *typeEntry, *tempConstr;
        BarConstrGen *tempBarConstrGen = barConstrGen;
        /* Insert both type and constr in st and push all constr
            given to the params vector of type */
        st.insert(id, new CustomType(), ENTRY_TYPE);
        typeEntry = st.getLastEntry();
        typeEntry->type->name = id;
        constr->sem();
        tempConstr = st.getLastEntry();
        /* Constructors of a user defined type */
        typeEntry->params.push_back(tempConstr);
        /* User defined type in a Constructor (single argument) */
        tempConstr->params.push_back(typeEntry);
        while (tempBarConstrGen != nullptr) {
            tempBarConstrGen->getConstr()->sem();
            tempConstr = st.getLastEntry();
            // bool isDuplicate = false;
            for (auto p : typeEntry->params) {
                if (p->id == tempConstr->id) {
                    /* Print Error - duplicate type color = Red | Red | Blue | Yellow */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err = new DuplicateEntry(tempConstr->id, false);
                    err->printError();
                    // isDuplicate = true;
                }
            }
            // if (!isDuplicate) {
                /* Constructors of a user defined type */
                typeEntry->params.push_back(tempConstr);
                /* User defined type in a Constructor (single argument) */
                tempConstr->params.push_back(typeEntry);
            // }
            tempBarConstrGen = tempBarConstrGen->getNext();
        }
    }

    virtual llvm::Value* compile() const override {

        SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
        if (se != nullptr) {
            constr->compile();
            if (barConstrGen != nullptr) barConstrGen->compile();
        }

        return nullptr;
    }

private:
    std::string id;
    Constr *constr;
    BarConstrGen *barConstrGen;
};

class TdefGen : public AST {
public:
    TdefGen(Tdef *td, TdefGen *tdg): tDef(td), tDefGen(tdg) {}

    virtual void printOn(std::ostream &out) const override {

        if (tDefGen == nullptr) {
            out << "TdefGen("; tDef->printOn(out); out << ")";
        }
        else {
            out << "TdefGen("; tDef->printOn(out); out << ", "; tDefGen->printOn(out); out << ")";
        }

    }

    TdefGen *getNext() { return tDefGen; }

    Tdef *getTdef() { return tDef; }

    virtual void sem() override {
        tDef->sem();
        if (tDefGen != nullptr) tDefGen->sem();
    }

    virtual llvm::Value* compile() const override {
        tDef->compile();
        if (tDefGen != nullptr) tDefGen->compile();

        return nullptr;
    }

private:
    Tdef *tDef;
    TdefGen *tDefGen;
};

class TypeDef : public Block {
public:
    TypeDef(Tdef *td, TdefGen *tdg): tDef(td), tDefGen(tdg) {}

    virtual void printOn(std::ostream &out) const override {

        if (tDefGen == nullptr) {
            out << "TypeDef("; tDef->printOn(out); out << ")";
        }
        else {
            out << "TypeDef("; tDef->printOn(out); out << ", "; tDefGen->printOn(out); out << ")";
        }

    }

    virtual void sem() override {
        tDef->sem();
        if (tDefGen != nullptr) tDefGen->sem();
        /* search CustomId params in Constr in types */
        SymbolEntry *tempEntry = st.lookup(tDef->getName());
        /* for every Constr object in user defined type */
        for (auto constr : tempEntry->params) {
            /* if its *type* params (Constr of {params}) are not empty */
            if (!dynamic_cast<CustomId*>(constr->type)->getParams().empty()) {
                int i = 0;
                /* for every *type* param */
                for (auto typeParam : dynamic_cast<CustomId*>(constr->type)->getParams()) {
                    /* if current *type* param is CustomId, make it point to user defined type, else it's not defined */
                    if (typeParam->typeValue == TYPE_ID) {
                        SymbolEntry *typeEntry = st.lookup(typeParam->name);
                        if (typeEntry != nullptr) dynamic_cast<CustomId*>(constr->type)->replaceParam(typeEntry->type, i);
                        else {
                            /* Print Error - unbound constructor */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new Error("Unbound constructor");
                            err->printMessage();
                        }
                    }
                    i++;
                }
            }
        }
    }

    void defineUDT(Tdef *td) const {
        pseudoST.incrSize();
        SymbolEntry *tdSE = currPseudoScope->lookup(td->getName(), pseudoST.getSize());
        if (tdSE != nullptr) {

            std::string udtName = tdSE->id;
            
            /* create udt */
            std::vector<llvm::Type *> members;
            /* tag */
            members.push_back(i32);

            /* create the udt */
            llvm::StructType *udtStruct = llvm::StructType::create(TheContext, udtName);
            udtStruct->setBody(members);

            tdSE->LLVMType = udtStruct;

            /* increment number of variables (Constr) */
            pseudoST.incrSize();

            /* increment number of variables (BarConstrGen) */
            BarConstrGen *bcg = td->getBarConstrGen();
            while (bcg != nullptr) {
                pseudoST.incrSize();
                bcg = bcg->getNext();
            }
        }
    }

    virtual llvm::Value* compile() const override {
        
        /* define first udt */
        defineUDT(tDef);

        /* declare all udt type a = ... and b = ... */
        if (tDefGen != nullptr) {
            TdefGen *tdg = tDefGen;
            while (tdg != nullptr) {
                defineUDT(tdg->getTdef());
                tdg = tdg->getNext();
            }
        }

        tDef->compile();
        if (tDefGen != nullptr) tDefGen->compile();

        return nullptr;
    }

private:
    Tdef *tDef;
    TdefGen *tDefGen;
};

#endif