#ifndef __AST_HPP__
#define __AST_HPP__

#include <cstring>
#include <iostream>
#include <vector>
#include "types.hpp"
#include "AST.hpp"
#include "library.hpp"
#include "error.hpp"

inline std::ostream & operator<< (std::ostream &out, const AST &ast) {
    ast.printOn(out);
    return out;
}

class Constant : public AST {};

class Expr : public AST {
public:

    /* Needed for Clause -> call from Match */
    // virtual Expr *sem_getClauseExpr(SymbolEntry *se) {}
    /* Needed for Classes Id | Constr | PatternConstr -> call from Match */
    virtual SymbolEntry *sem_getExprObj() { return nullptr; }

    CustomType *getType() { return type; }

    void setType(CustomType *t) { this->type = t; }

    virtual std::pair<CustomType *, int> getRefFinalType(CustomType *ct) {

        int levels = 1;
        CustomType *obj = ct;
        
        while (obj->ofType != nullptr && obj->typeValue == TYPE_REF) {
            levels++;
            obj = obj->ofType;
        }
        if (levels == 1) return std::make_pair(ct, levels);
        return std::make_pair(obj, levels);

    }

protected:
CustomType *type;
};

class Pattern : public Expr {};

class Block : public AST {
public:
    Block(): block() {}

    void appendBlock(Block *b){
        block.push_back(b);
    }

    virtual void printOn(std::ostream &out) const override {
        
        out << "Block(";
        bool first = true;
        for(auto i = block.rbegin(); i != block.rend(); ++i) {
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
        for(auto i = block.rbegin(); i != block.rend(); ++i) (*i)->sem();
        st.closeScope();
        st.closeScope();
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
            out <<")";
        }
        else { 
            out << "Id(" << name <<", "; expr->printOn(out); out <<", "; exprGen->printOn(out); out <<")";
        }
    }

    std::string getName() { return name; }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(name); }

    virtual void sem() override {
        // std::cout << "Im in Id for " << name << std::endl;

        /* lookup for variable, if exists [might need to be moved down] */
        if (expr == nullptr && exprGen == nullptr) {
            // std::cout << "Correctly in Id\n";
            SymbolEntry *tempEntry = st.lookup(name);
            // std::cout << "Correctly in Id after lookup\n";
            if (tempEntry != nullptr) this->type = tempEntry->type;
            else { 
                /* Print Error First Occurance*/ 
                this->type = new Unknown();
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new FirstOccurence(name);
                err->printError();
                st.insert(name, new Unknown(), ENTRY_TEMP);
            }
        }
        /* lookup for function */
        else {
            // st.printST();
            // std::cout << "Im in Id - call function " << name << std::endl;
            SymbolEntry *tempEntry = st.lookup(name);

            if (tempEntry != nullptr) {

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
                    if (expr->getType()->typeValue == TYPE_CUSTOM){
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
                    else if (expr->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = tempName; }
                }
                if (tempEntry->params.front()->type->typeValue != TYPE_UNKNOWN && expr->getType()->typeValue == TYPE_UNKNOWN) {
                    expr->setType(tempEntry->params.front()->type);
                    SymbolEntry *se = expr->sem_getExprObj();
                    if (se->type->typeValue == TYPE_REF || se->type->typeValue == TYPE_ARRAY) se->type->ofType = expr->getType();
                    else se->type = expr->getType();
                }

                if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->getType()->typeValue == TYPE_UNKNOWN) {
                    expr->setType(tempEntry->params.front()->type);
                    SymbolEntry *se = expr->sem_getExprObj();
                    if (se->type->typeValue == TYPE_REF || se->type->typeValue == TYPE_ARRAY) se->type->ofType = expr->getType();
                    else se->type = expr->getType();
                } 

                /* Check first param of function with given param */
                if (expr->getType()->typeValue != tempEntry->params.front()->type->typeValue) { 
                    /* edge case for ref(unknown) to array(unknown) -> type correction */
                    if(expr->getType()->typeValue == TYPE_ARRAY && expr->getType()->ofType->typeValue == TYPE_UNKNOWN
                    && tempEntry->params.front()->type->typeValue == TYPE_REF && tempEntry->params.front()->type->ofType->typeValue == TYPE_UNKNOWN){
                        tempEntry->params.front()->type = expr->getType();
                    }
                    else{
                        /* Print Error - type mismatch */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(expr->getType(), tempEntry->params.front()->type);
                        err->printError();
                    }
                }
                else {
                    if (expr->getType()->typeValue == TYPE_ARRAY 
                    && tempEntry->params.front()->type->typeValue == TYPE_ARRAY 
                    && expr->getType()->size != tempEntry->params.front()->type->size) {
                        /* Print Error - type mismatch */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new ArrayDimensions(dynamic_cast<Array *>(expr->getType()), dynamic_cast<Array *>(tempEntry->params.front()->type));;
                        err->printError();
                    }
                }
                if (expr->getType()->typeValue == TYPE_FUNC) {}

                /* lookup for the rest params of a function, if they exist */
                if (exprGen != nullptr) {
                    exprGen->sem();
                    /* Check for the rest params of function with the rest given params */
                    ExprGen *tempExprGen = exprGen;
                    long unsigned int i = 1;
                    for (; (i < tempEntry->params.size() && tempExprGen != nullptr); i++, tempExprGen = tempExprGen->getNext()) {
                        /* Check if both function param and given param have unknown type */
                        if (tempEntry->params.at(i)->type->typeValue == TYPE_UNKNOWN && tempExprGen->getType()->typeValue == TYPE_UNKNOWN) { 
                            /* Warning polymorphic value */ 
                            SymbolEntry *exprGenEntry = tempExprGen->getExpr()->sem_getExprObj();
                            tempEntry->params.at(i)->type = exprGenEntry->type;

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
                            else if (tempExprGen->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
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
                            else if (tempEntry->params.at(i)->type->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
                        }
                        /* Check ith param given that has the same type as the ith param of the function */
                        else {
                            if (tempExprGen->getType()->typeValue == TYPE_ID 
                            && tempEntry->params.at(i)->type->typeValue == TYPE_CUSTOM 
                            && tempEntry->params.at(i)->type->name != tempExprGen->getExpr()->sem_getExprObj()->params.front()->type->name) { 
                                /* Print Error - type mismatch */ 
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(tempEntry->params.at(i)->type, tempExprGen->getExpr()->sem_getExprObj()->params.front()->type);
                                err->printError();
                            }
                            else if (tempExprGen->getType()->typeValue != TYPE_ID && tempEntry->params.at(i)->type->typeValue != TYPE_CUSTOM) {
                                if (tempEntry->params.at(i)->type->typeValue != tempExprGen->getType()->typeValue) { 
                                    /* edge case for ref(unknown) to array(unknown) -> type correction */
                                    if(tempExprGen->getType()->typeValue == TYPE_ARRAY && tempExprGen->getType()->ofType->typeValue == TYPE_UNKNOWN
                                    && tempEntry->params.at(i)->type->typeValue == TYPE_REF && tempEntry->params.at(i)->type->ofType->typeValue == TYPE_UNKNOWN){
                                        tempEntry->params.at(i)->type = expr->getType();
                                    }
                                    else{
                                        /* Print Error - type mismatch */ 
                                        std::cout << "Line " <<__LINE__ << " -> ";
                                        Error *err = new TypeMismatch(tempEntry->params.at(i)->type, tempExprGen->getType());
                                        err->printError();
                                    }
                                }
                                else {
                                    if (tempExprGen->getType()->typeValue == TYPE_ARRAY 
                                    && tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY 
                                    && tempExprGen->getType()->size != tempEntry->params.at(i)->type->size) {
                                        /* Print Error - type mismatch */
                                        std::cout << "Line " <<__LINE__ << " -> ";
                                        Error *err = new ArrayDimensions(dynamic_cast<Array *>(tempExprGen->getType()), dynamic_cast<Array *>(tempEntry->params.at(i)->type));;
                                        err->printError();
                                    }
                                }
                            }
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
                                    /* Print Error - function param ref(ref(ref(unknown))), called param ref(int) */ 
                                    // not implemented yet
                                }
                                else tempEntry->params.at(i)->type = tempExprGen->getType();
                            }
                            else if (pairExpr2.first->typeValue == TYPE_UNKNOWN) { 
                                if (pairExpr1.second < pairExpr2.second) { 
                                    /* Print Error - called param ref(ref(ref(unknown))), function param ref(int) */ 
                                    // not implemented yet
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
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(pairExpr1.first, pairExpr2.first);
                                err->printError();
                            }
                        }
                    }
                    int extraParams = 0;
                    CustomType *lastTypeMore;
                    CustomType *lastTypeLess;
                    std::vector<CustomType *> params;
                    if(i < tempEntry->params.size()) {
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
                        // not implemented yet
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
                        // not implemented yet
                        extraParams--;
                        params.push_back(tempEntry->params.at(i)->type);
                        i++;
                    }
                        // not implemented yet -> SML printing? (int with int->int)
                        /* Params given are more than expected */
                        if(extraParams > 0) {
                            CustomType *newFunc = new Function(new Unknown());
                            for(int j = 0; j < extraParams+1; j++){
                                dynamic_cast<Function *>(newFunc)->params.push_back(params.at(j));
                            }
                            /* Print Error */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new TypeMismatch(tempEntry->params.back()->type, newFunc);
                            err->printError();
                        }
                        /* Params given are less than expected */
                        if(extraParams < 0) {
                            CustomType *newFunc = new Function(new Unknown());
                            for(int j = 0; j < -(extraParams-1); j++){
                                dynamic_cast<Function *>(newFunc)->params.push_back(params.at(j));
                            }
                            /* Print Error */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err;
                            if(tempExprGen != nullptr) err = new TypeMismatch(newFunc, tempExprGen->getType());
                            else err = new TypeMismatch(newFunc, expr->getType());
                            err->printError();
                        }
                }
                /* edge case - when calling a function with exactly 1 param but more are required */
                else {
                    if(tempEntry->params.size() > 1) {
                        std::vector<CustomType *> params;
                        CustomType *newFunc = new Function(new Unknown());
                        for(long unsigned int j = 0; j < tempEntry->params.size(); j++)
                            dynamic_cast<Function *>(newFunc)->params.push_back(tempEntry->params.at(j)->type);
                        /* Print Error */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err;
                        err = new TypeMismatch(newFunc, expr->getType());
                        err->printError();
                    }
                }
            }
            else { 
                /* Print Error First Occurance */
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new FirstOccurence(name);
                err->printError();
            }
            // std::cout << "Exiting function sem (ID) " << std::endl;
        }
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

    std::string getName() { return name; }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(name); }

    virtual void sem() override {
        /* lookup for variable if exists, otherwise create  */
        SymbolEntry *tempEntry = st.lookup(name);
        if (tempEntry != nullptr) this->type = tempEntry->type;
        else { 
            CustomType *ct = new Unknown();
            st.insert(name, ct, ENTRY_VARIABLE);
            this->type = ct;
        }
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
            /* Print Error - first occurance */ 
            this->type = new Unknown();
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new FirstOccurence(Id);
            err->printError();
        }
        else this->type = tempEntry->type;    
        if (patternGen != nullptr) {
            patternGen->sem();
            if(tempEntry != nullptr) {
                PatternGen *tempPatternGen = patternGen;
                int index = 0;
                SymbolEntry *patternEntry;
                while (tempPatternGen != nullptr){

                    /* get SymbolEntry of given param, if it's nullptr, means it's a PatternId, therefore do type inference */
                    patternEntry = tempPatternGen->sem_getExprObj();
                    if(patternEntry != nullptr)
                        patternEntry->type = dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index);
                    // type check
                    // might need to adjust for customtypes
                    if(tempPatternGen->getType()->typeValue != TYPE_UNKNOWN && tempPatternGen->getType()->typeValue != dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index)->typeValue) {
                        std::cout << "Line " <<__LINE__ << " -> ";
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

    /* this symbol entry has name attribute and val array with X positions of types given -> create by Constr */
    // virtual Expr *sem_getClauseExpr(SymbolEntry *se) override {

    //     // compare se.name to p.name if true then compare se.val[] to p.val[] if true expr->sem()
    //     if (se->id == pattern->sem_getExprObj()->id) {
    //         // value check if true 
    //         return expr;
    //     }
    //     return nullptr;
    // }

    virtual void sem() override {
        pattern->sem();
        expr->sem(); 
        // st.printST();
        this->type = expr->getType();
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

    // virtual Expr *sem(SymbolEntry *se) {
    //     /* if reached here, more clauses exist */
        
    //     /* pass the tempEntry to clause in order to compare it with every pattern */
    //     /* in case that matched the expression will be returned else nullptr will be returned */
    //     Expr *returnedExpr = clause->sem_getClauseExpr(se);
    //     if (!returnedExpr) { return returnedExpr; }
    //     return nullptr;
    // }

    virtual void sem() override {
        clause->sem();
        this->type = clause->getType();
        if (barClauseGen != nullptr) barClauseGen->sem();
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
    virtual void sem() override {
        expr->sem();
        clause->sem();
        /* if type of expression is unknown, set it to type of clause */
        SymbolEntry *exprEntry = expr->sem_getExprObj();
        if (expr->getType()->typeValue == TYPE_UNKNOWN && clause->getPattern()->getType() != nullptr && clause->getPattern()->getType()->typeValue != TYPE_UNKNOWN) {
            if (clause->getPattern()->getType()->typeValue != TYPE_ID) exprEntry->type = clause->getPattern()->getType();
            else exprEntry->type = clause->getPattern()->sem_getExprObj()->params.front()->type;
        }
        /* might need to remove below if and assign this->type even if clause doesn't have a type yet */
        if(clause->getType() != nullptr)
            this->type = clause->getType();
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
                    /* type check clause type != expr type */
                    SymbolEntry *clauseObj = tempBarClauseGen->getClause()->sem_getExprObj();
                    if (clausePatternType->typeValue == TYPE_ID 
                    && clauseObj != nullptr && clauseObj->params.front()->type != exprEntry->type) { 
                        /* Print Error - cannot unify a with b */ 
                        // might need re-check
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(clauseObj->params.front()->type, exprEntry->type);
                        err->printError();
                    }
                    /* type inference for clause return type */
                    if (prev->typeValue == TYPE_UNKNOWN && tempBarClauseGen->getType()->typeValue != TYPE_UNKNOWN) { 
                        /* Change previous clause type according to current clause type */
                        if(prev->typeValue == TYPE_UNKNOWN) {
                            if(tempBarClauseGen->getType()->typeValue == TYPE_ID)
                                prev = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj()->params.front()->type;
                            else prev = tempBarClauseGen->getType();
                        }
                        /* Change previous clause SymbolEntry (if it exists) according to current clause type */
                        if(prevSE != nullptr && prevSE->type != nullptr){
                            if (prevSE->type->typeValue == TYPE_UNKNOWN){
                                if(tempBarClauseGen->getType()->typeValue == TYPE_ID)
                                    prevSE->type = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj()->params.front()->type;
                                else prevSE->type = tempBarClauseGen->getType();
                            }
                        }
                    }
                    /* type check all clause exprs (need to be the same type) */
                    if (prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                        /* if prev is a constructor and current is a customtype */
                        if(prev->typeValue == TYPE_ID && tempBarClauseGen->getType()->typeValue == TYPE_CUSTOM && dynamic_cast<CustomType *>(prevSE->params.front()->type)->name != tempBarClauseGen->getType()->name){
                            /* Print Error - cannot unify a with b */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                        /* if prev is a customtype and current is a constructor */
                        else if(prev->typeValue == TYPE_CUSTOM && tempBarClauseGen->getType()->typeValue == TYPE_ID && prev->name != tempBarClauseGen->getClause()->getExpr()->sem_getExprObj()->params.front()->type->name){
                            /* Print Error - cannot unify a with b */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                        /* if both are constructors */
                        else if(prev->typeValue == TYPE_ID && tempBarClauseGen->getType()->typeValue == TYPE_ID 
                        && dynamic_cast<CustomType *>(prevSE->params.front()->type)->name != tempBarClauseGen->getClause()->getExpr()->sem_getExprObj()->params.front()->type->name){
                            /* Print Error - cannot unify a with b */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                        /* if both are customtypes */
                        else if(prev->typeValue == TYPE_CUSTOM && tempBarClauseGen->getType()->typeValue == TYPE_CUSTOM
                        && prev->name != tempBarClauseGen->getType()->name){
                            /* Print Error - cannot unify a with b */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                        /* if prev is unrelated to types */
                        else if((prev->typeValue != TYPE_ID && prev->typeValue != TYPE_CUSTOM) && prev->typeValue != tempBarClauseGen->getType()->typeValue){
                            /* Print Error - cannot unify a with b */
                            /* if current clause is a constructor while prev wasn't */
                            if(tempBarClauseGen->getType()->typeValue == TYPE_ID && tempBarClauseGen->getClause()->sem_getExprObj() != nullptr){
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(prev, tempBarClauseGen->getClause()->sem_getExprObj()->params.front()->type);
                                err->printError();
                                
                            }
                            /* if both clauses aren't constructors */
                            else {
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                                err->printError();
                            }
                        }
                        /* if current is unrelated to types */
                        else if((tempBarClauseGen->getType()->typeValue != TYPE_ID && tempBarClauseGen->getType()->typeValue != TYPE_CUSTOM) && prev->typeValue != tempBarClauseGen->getType()->typeValue){
                            /* Print Error - cannot unify a with b */
                            /* if current clause is a constructor while prev wasn't */
                            if(prev->typeValue == TYPE_ID && prevSE != nullptr){
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(dynamic_cast<CustomType *>(prevSE->params.front()->type), tempBarClauseGen->getType());
                                err->printError();
                                
                            }
                            /* if both clauses aren't constructors */
                            else {
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                                err->printError();
                            }
                        }
                    }    
                }
                /* in case that expr does not have TYPE_ID */
                else {
                    if ((exprEntry->type->typeValue == TYPE_UNKNOWN && clausePatternType->typeValue != TYPE_UNKNOWN) 
                     || (exprEntry->type->typeValue != TYPE_UNKNOWN && clausePatternType->typeValue == TYPE_ID)) 
                        exprEntry->type = clause->getPattern()->getType();
                }

                // might need something like the following, but since it changes pointers, somewhat different
                if(this->type == nullptr) {
                    if(prev != nullptr && prev->typeValue == TYPE_ID) {
                        if(prevSE != nullptr)
                            this->type = prevSE->params.front()->type;
                    }
                    else this->type = prev;
                }
                else if(prev != nullptr && prev->typeValue != TYPE_UNKNOWN) {
                    if(prev->typeValue == TYPE_ID){
                        this->type = prevSE->params.front()->type;
                    }
                    else this->type = prev;
                }
                /* move pointers */
                prev = tempBarClauseGen->getType();
                prevSE = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj();
                tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
            }

            if (exprEntry->type->typeValue != TYPE_UNKNOWN) {

                /* type inference for first clause object */
                if(clause->sem_getExprObj() != nullptr && clause->sem_getExprObj()->type->typeValue == TYPE_UNKNOWN) {
                    SymbolEntry *firstClauseObj = clause->sem_getExprObj();
                    firstClauseObj->type = exprEntry->type;
                }

                /* if clause returns something other than Unknown or CustomId */
                if (clause->getPattern()->getType()->typeValue != TYPE_ID && clause->getPattern()->getType()->typeValue != TYPE_UNKNOWN){
                    /* if expr is something other than CustomId -> type mismatch CustomId(expr) with !CustomId(clause) */
                    if (exprEntry->type->typeValue == TYPE_ID) { 
                        /* Print Error - cannot unify exprEntry->type->id with clause->getPattern()->getType()->typeValue */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType()); 
                        err->printError(); // same message?
                    }
                    else if(exprEntry->type->typeValue != clause->getPattern()->getType()->typeValue) {
                        /* Print Error - cannot unify exprEntry->type->typeValue with clause->getPattern()->getType()->typeValue */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType()); 
                        err->printError(); // same message?
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
                        if (clausePatternType->typeValue != TYPE_ID && clausePatternType->typeValue != TYPE_UNKNOWN)
                            if (exprEntry->type->typeValue != clausePatternType->typeValue) { 
                                /* Print Error - cannot unify a with b */
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(exprEntry->type, clausePatternType);
                                err->printError();
                            }
                    }
                    else if (clausePatternType->typeValue != TYPE_ID && clausePatternType->typeValue != TYPE_UNKNOWN) { 
                        /* Print Error - cannot unify exprEntry->type->id with clause->getPattern()->getType()->typeValue */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                        err->printError();
                    }
                    
                    /* move pointers */
                    tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
                    /* type inference for return type of clause */
                    if(clauseType->typeValue == TYPE_UNKNOWN) clauseType = exprEntry->type;
                }
            }
        }
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
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new Expectation(expectedType, start->getType());
            err->printError();
        }
        end->sem();
        if (end->getType()->typeValue != TYPE_INT) { 
            /* Print Error */
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Integer());
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new Expectation(expectedType, end->getType());
            err->printError();
        }
        /* if everything ok then proceed */
        expr->sem();
        this->type = expr->getType();
        st.closeScope();
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
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Boolean());
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new Expectation(expectedType, loopCondition->getType());
            err->printError();
        }
        expr->sem();
        this->type = expr->getType();
        st.closeScope();
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
            out << "If("; condition->printOn(out); out <<", "; expr1->printOn(out); out <<")";
        }
        else {
            out << "If("; condition->printOn(out); out <<", "; expr1->printOn(out); out <<", "; expr2->printOn(out); out <<")";
        }
    }

    virtual void sem() override {
        condition->sem();
        if (condition->getType()->typeValue != TYPE_BOOL) { 
            /* print Error */ 
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Boolean());
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new Expectation(expectedType, condition->getType());
            err->printError();
        }
        expr1->sem();

        if(expr2 != nullptr) {
            expr2->sem();
            /* might need to reconsider expr1 and expr2 getType()->typeValue == TYPE_UNKNOWN */
            if (expr1->getType()->typeValue == TYPE_UNKNOWN) expr1->setType(expr2->getType());
            else if (expr2->getType()->typeValue == TYPE_UNKNOWN) expr2->setType(expr1->getType());
            /* expr1 and expr2 must be of same type */
            else if (expr1->getType()->typeValue != expr2->getType()->typeValue) { 
                /* print Error */ 
                // might need some re-thinking
                /* if expr1 is a constructor and expr2 is a custom type */
                if(expr1->getType()-> typeValue == TYPE_ID && expr2->getType()->typeValue == TYPE_CUSTOM
                && expr1->sem_getExprObj() != nullptr && expr1->sem_getExprObj()->params.front()->type->name != expr2->getType()->name) {
                    std::cout << "Line " <<__LINE__ << " -> ";
                    Error *err = new TypeMismatch(expr1->sem_getExprObj()->params.front()->type, expr2->getType());
                    err->printError();
                }
                /* if expr1 is a constructor and expr2 is anything *but* a custom type */
                else if(expr1->getType()-> typeValue == TYPE_ID && expr2->getType()->typeValue != TYPE_CUSTOM
                && expr1->sem_getExprObj() != nullptr) {
                    std::cout << "Line " <<__LINE__ << " -> ";
                    Error *err = new TypeMismatch(expr1->sem_getExprObj()->params.front()->type, expr2->getType());
                    err->printError();
                }
                /* if expr1 is not a constructor and expr2 is a custom type */
                else if(expr1->getType()-> typeValue != TYPE_ID && expr2->getType()->typeValue == TYPE_CUSTOM) {
                    std::cout << "Line " <<__LINE__ << " -> ";
                    Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                    err->printError();
                }
                /* if both are unrelated to types */
                else if(expr1->getType()->typeValue != TYPE_ID && expr2->getType()->typeValue != TYPE_CUSTOM) {
                    std::cout << "Line " <<__LINE__ << " -> ";
                    Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                    err->printError();
                }
            }
        }
        if(expr1->getType()->typeValue != TYPE_ID) this->type = expr1->getType();
        else {
            SymbolEntry *tempEntry = expr1->sem_getExprObj();
            this->type = tempEntry->params.front()->type;
        }

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
        st.openScope();
        expr->sem();
        this->type = expr->getType(); 
        st.closeScope();
    }

private:
Expr *expr;
};

class CommaExprGen : public AST {
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
        if (expr->getType()->typeValue != TYPE_INT) { /* Throw Error */ }
        if(commaExprGen != nullptr) commaExprGen->sem();
    }

private:
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class Par : public AST {
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
        newParamEntry->entryType = ENTRY_PARAMETER;
        tempEntry->params.push_back(newParamEntry);
    }

private:
std::string id;
CustomType *type;
};

class ParGen : public AST {
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
private:
    Par *par;
    ParGen *parGen;
};

class Def : public AST {
public:
    Def(char *id, ParGen *pg, Expr *e, CustomType *t, CommaExprGen *ceg, bool isMutable): 
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
                expr->sem();
                if (expr->getType()->typeValue != TYPE_INT) { /* Throw Error */ }
                if (commaExprGen != nullptr) commaExprGen->sem();
                
                /* get dimensions by iterating commaExprGen "list" */
                int dimensions = 1;
                CommaExprGen *tempExpr = commaExprGen;
                while (tempExpr != nullptr) {
                    dimensions++;
                    tempExpr = tempExpr->getNext();
                }
                
                /* array's type is given */
                if (type != nullptr) st.insert(id, new Array(type, dimensions), ENTRY_VARIABLE);
                /* array's type is unknown */
                else st.insert(id, new Array(new Unknown(), dimensions), ENTRY_VARIABLE);
            }
        }
        else {
            /* if def is a non mutable variable - constant */
            if (parGen == nullptr) {
                expr->sem();
                /* not null type */
                if (type != nullptr) {
                    /* check if type given is same as expression's */
                    if (type->typeValue == expr->getType()->typeValue) st.insert(id, type, ENTRY_CONSTANT);
                    /* not equal => Error */
                    else { /* Throw Error */ }
                }
                /* null type means that type is equal to expression's */
                else {
                    /* expression's type might be of type ref so we need to save the entry in st as entry variable and not constant (is mutable) */
                    if (expr->getType()->typeValue != TYPE_REF) st.insert(id, expr->getType(), ENTRY_CONSTANT);
                    else st.insert(id, expr->getType(), ENTRY_VARIABLE);
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
                    // std::cout << "Inserting \"" <<i->id <<"\" in new scope, MEM: " <<i <<std::endl;
                    st.insert(i->id, i);
                }
                expr->sem();
                dynamic_cast<Function*>(funcEntry->type)->outputType = expr->getType();
                this->type = funcEntry->type;
                st.closeScope();
            }
        }
    }

private:
    char *id;
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

private:
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
        // might need rec param to def
        // std::cout << "Im in Let" << std::endl;
        def->sem();
        if (defGen != nullptr) defGen->sem();
    }

private:
    Def *def;
    DefGen *defGen;
    bool rec;
};

class LetIn : public Expr, public Block {
public:
    LetIn(Let* l, Expr *e): let(l), expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "LetIn("; let->printOn(out); out <<", "; expr->printOn(out); out << ")";
    }

    virtual void sem() override {
        st.openScope();
        let->sem();
        expr->sem();
        this->type = expr->getType();
        st.closeScope();
    }

private:
Let *let;
Expr *expr;
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
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Reference(new Unknown()));
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = new Unit();
    }

private:
Expr *expr;
};

class New : public Expr {
public:
    New(CustomType *t): type(t) {}

    virtual void printOn(std::ostream &out) const override {
        out << "New("; type->printOn(out); out << ")";
    }

    virtual void sem() override { this->type = new Reference(type); }

private:
CustomType *type;
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

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(id); }

    virtual void sem() override {
        // std::cout << " ENTERING ARRAYITEM " << std::endl;
        SymbolEntry *tempEntry = st.lookup(ENTRY_VARIABLE, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_PARAMETER, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_TEMP, id);
        if (tempEntry == nullptr) {
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new FirstOccurence(id);
            err->printError();
            tempEntry = new SymbolEntry(id, new Unknown());
            tempEntry->entryType = ENTRY_TEMP;
            st.insert(id, new Unknown(), ENTRY_TEMP);
        }

        if (tempEntry->type->typeValue == TYPE_ARRAY && dynamic_cast<Array *>(tempEntry->type)->isInferred) dynamic_cast<Array *>(tempEntry->type)->isInferred = false;

        /* type inference */
        if (tempEntry->entryType != ENTRY_TEMP && tempEntry->type->typeValue == TYPE_UNKNOWN) tempEntry->type = new Array(new Unknown(), 1);
        
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
        if (expr->getType()->typeValue != TYPE_INT) { /* Throw Error */ }
        
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
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new ArrayTypeMismatch(dimensions, new Array(new Unknown(), dimensions), tempEntry->type);
            err->printError();
        }
        else {
            if (dimensions >= tempEntry->type->size) { /* all ok */ }
            else { 
                /*  Print Error 
                    type mismatch in expression,
                    a should be an array of 1 dimensions,
                    impossible to unify int with @3
                */ 
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new ArrayDimensions(new Array(new Unknown(), dimensions), dynamic_cast<Array *>(tempEntry->type));
                err->printError();
            }
        }
        if(tempEntry->type->typeValue != TYPE_UNKNOWN) this->type = tempEntry->type;
        else this->type = new Array(new Unknown(), dimensions);
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
    
    virtual void printOn(std::ostream &out) const override { out << "Dim("<< id <<", " << intconst <<")"; }

    virtual void sem() override {
        // std::cout << "In Dim\n";
        SymbolEntry *tempEntry = st.lookup(ENTRY_VARIABLE, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_PARAMETER, id);
        if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_TEMP, id);
        if (tempEntry == nullptr) {
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new FirstOccurence(id);
            err->printError();
            tempEntry = new SymbolEntry(id, new Unknown());
            tempEntry->entryType = ENTRY_TEMP;
            st.insert(id, new Unknown(), ENTRY_TEMP);
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
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new ArrayDimensions(new Array(new Unknown(), intconst), dynamic_cast<Array *>(tempEntry->type));
                err->printError();
            }
        }
        else { 
            /* Print Error - Impossible to unify type given (symbolentry's type) with array */
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new ArrayTypeMismatch(intconst, new Array(new Unknown(), intconst), tempEntry->type);
            err->printError();
        }
    }

private:
std::string id;
int intconst;
};

class BinOp : public Expr {
public:
    BinOp(Expr *e1, const char * op, Expr *e2): expr1(e1), op(op), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {
        out << "BinOp("; expr1->printOn(out); out <<", " << op <<", "; expr2->printOn(out); out <<")";
    }

    virtual void sem() override {

        // std::cout << "Entering BinOP for op(" << op <<")" << std::endl;
        expr1->sem();
        // std::cout << "After expr1->sem " << std::endl;
        expr2->sem();
        // std::cout << "After expr2->sem " << std::endl;
        SymbolEntry *tempExpr1 = expr1->sem_getExprObj();
        SymbolEntry *tempExpr2 = expr2->sem_getExprObj();

        /* type inference for all binops exept ';' & ':=' */
        if (strcmp(op, ";") && strcmp(op, ":=")) {
            
            /* both unknown */
            if (tempExpr1 != nullptr && tempExpr2 != nullptr 
             && tempExpr1->entryType != ENTRY_TEMP && tempExpr2->entryType != ENTRY_TEMP 
             && expr1->getType()->typeValue == TYPE_UNKNOWN && expr2->getType()->typeValue == TYPE_UNKNOWN) {
                expr1->setType(expr2->getType());
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
                    else { /* for now nothing */ }
                    
                    if (tempExpr2->type->typeValue != TYPE_FUNC) {
                        if (!(tempExpr2->type->typeValue == TYPE_ARRAY || tempExpr2->type->typeValue == TYPE_REF)) tempExpr2->type = expr2->getType();
                        else tempExpr2->type->ofType = expr2->getType(); 
                    }
                }
            }
        }

        if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "mod")) {
            this->type = new Integer();

            /* type inference - if both are unknown */
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                // expr1->setType(new Integer());
                expr1->getType()->~CustomType();
                expr1->setType(new (expr1->getType()) Integer());
            }

            if (expr1->getType()->typeValue == TYPE_INT && expr2->getType()->typeValue == TYPE_INT) {}
            else { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Integer());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err;
                if (expr1->getType()->typeValue != TYPE_INT) err = new Expectation(expectedType, expr1->getType());
                if (expr2->getType()->typeValue != TYPE_INT) err = new Expectation(expectedType, expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, "+.") || !strcmp(op, "-.") || !strcmp(op, "*.") || !strcmp(op, "/.") || !strcmp(op, "**")) {
            this->type = new Float();

            /* type inference - if both are unknown */
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                // expr1->setType(new Float());
                expr1->getType()->~CustomType();
                expr1->setType(new (expr1->getType()) Float());
            }
            if (expr1->getType()->typeValue == TYPE_FLOAT && expr2->getType()->typeValue == TYPE_FLOAT) {}
            else { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Float());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err;
                if (expr1->getType()->typeValue != TYPE_FLOAT) err = new Expectation(expectedType, expr1->getType());
                if (expr2->getType()->typeValue != TYPE_FLOAT) err = new Expectation(expectedType, expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, "=") || !strcmp(op, "<>")) {            
            /* the result will always be boolean */
            this->type = new Boolean();

            if (expr1->getType()->typeValue == expr2->getType()->typeValue 
             && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {
                // compare values
            }
            else { 
                /* Print Error */
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, "==") || !strcmp(op, "!=")) {
            this->type = new Boolean();
            /* type check */
            if (expr1->getType()->typeValue == expr2->getType()->typeValue
             && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {
                // value check
            }
            else { 
                /* Print Error */
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        } 
        else if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, ">=") || !strcmp(op, "<=")) {
            this->type = new Boolean();
            if (expr1->getType()->typeValue == expr2->getType()->typeValue 
             && (expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT 
             || expr1->getType()->typeValue == TYPE_CHAR || expr1->getType()->typeValue == TYPE_UNKNOWN)) {
                // value check
            }
            else { 
                /* Print Error */
                std::vector<CustomType *> expectedTypes;
                expectedTypes.push_back(new Integer());
                expectedTypes.push_back(new Float());
                expectedTypes.push_back(new Character());
                std::cout << "Line " <<__LINE__ << " -> ";
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
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (!strcmp(op, ";")) {
            this->type = expr2->getType();
        }
        else if (!strcmp(op, ":=")) {
            bool recursiveRefError = false;
            /* type inference */
            SymbolEntry *tempEntry = expr1->sem_getExprObj();
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_ARRAY && expr1->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                expr1->getType()->ofType = expr2->getType();
                tempEntry->type->ofType = expr2->getType();
            }
            else {
                if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                    if (expr2->getType()->typeValue != TYPE_ID) {
                        if (tempExpr2 != nullptr) {
                            // expr1->setType(new Reference(expr2->getType()));
                            std::pair <CustomType *, int> pairExpr1, pairExpr2;
                            pairExpr1 = expr1->getRefFinalType(tempExpr1->type);
                            pairExpr2 = expr2->getRefFinalType(tempExpr2->type);
                            if (pairExpr1.first != pairExpr2.first) {
                                /* change SumbolEntry type */
                                CustomType *tempCT = tempExpr1->type;
                                tempCT->~CustomType();
                                tempCT = new (tempCT) Reference(tempExpr2->type);
                                /* Change expr type */
                                expr1->setType(new Reference(expr2->getType()));
                            }
                            else {
                                recursiveRefError = true;
                                Error *err = new TypeMismatch(tempExpr1->type, tempExpr2->type);
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
                    else {
                        SymbolEntry *expr2_Entry = expr2->sem_getExprObj();
                        expr1->setType(expr2_Entry->params.front()->type);
                    }
                }
                // if (tempEntry->type->typeValue != TYPE_FUNC) tempEntry->type = expr1->getType();
            }

            this->type = new Unit();
            if (tempEntry != nullptr) {
                // if expr1 = Ref(Unknown) then replace Unknown with expr2 type
                if (expr1->getType()->typeValue == TYPE_REF && expr1->getType()->ofType->typeValue == TYPE_UNKNOWN){
                    if (expr2->getType()->typeValue != TYPE_ID) expr1->getType()->ofType = expr2->getType();
                    else {
                        SymbolEntry *expr2_Entry = expr2->sem_getExprObj();
                        expr1->getType()->ofType = expr2_Entry->params.front()->type;
                    }
                }
                else 
                if (expr1->getType()->typeValue == TYPE_ARRAY) {
                    if (expr1->getType()->ofType->typeValue != expr2->getType()->typeValue) { 
                        /* Print Error - type mismatch */
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                        err->printError();
                    }
                }
                // expr1 already has a ref type so need to compare type with expr2 type
                else {
                    std::pair <CustomType *, int> pairExpr1, pairExpr2;

                    if (expr1->getType()->typeValue == TYPE_REF) pairExpr1 = getRefFinalType(expr1->getType()->ofType);
                    if (expr2->getType()->typeValue == TYPE_REF) pairExpr2 = getRefFinalType(expr2->getType());
                    
                    if (!recursiveRefError) {
                        if (expr1->getType()->ofType->typeValue == expr2->getType()->typeValue) {

                            if (expr2->getType()->typeValue == TYPE_REF) {
                                if (pairExpr1.first->typeValue == pairExpr2.first->typeValue && pairExpr1.second == pairExpr2.second) {
                                    // if matches then value should be changed
                                }
                                else { 
                                    /* Print Error */
                                    std::cout << "Line " <<__LINE__ << " -> ";
                                    Error *err = new TypeMismatch(pairExpr1.first, pairExpr2.first);
                                    err->printError();
                                }
                            }
                            else { /* if matches then value should be changed */ }

                        }
                        else { 
                            /* Print Error - type mismatch */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                            err->printError();
                        }
                    }
                }
            }
            else { 
                /* Print Error - var not exist (first occurance) */
                // might need to check class since it's not 100% an Id
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new FirstOccurence(dynamic_cast<Id *>(expr1)->getName());
                err->printError();
            }
        }

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

    virtual void sem() override {
        // std::cout << " ENTERING UNOP " << std::endl;
        expr->sem();
        if (!strcmp(op, "!")) {
            /* If expr is Ref(type), make Dereference (convert Ref(type) to type) */
            /* or if expr is Array(type), make this type eq to array oftype */
            if (expr->getType()->typeValue == TYPE_REF || expr->getType()->typeValue == TYPE_ARRAY) {
                this->type = expr->getType()->ofType;
            }
            /* If expr is not Ref(type) - reached the end of References - make an Invalid type Type(Ref(nullptr)) */
            else {
                if (expr->getType()->typeValue == TYPE_UNKNOWN) {
                    expr->setType(new Reference(new Unknown()));
                    this->type = expr->getType()->ofType;
                }
                else {
                    this->type = expr->getType();
                    this->type->ofType = new Reference(this->type->ofType);
                }
            }
        }
        else if (!strcmp(op, "+")) {
            if (expr->getType()->typeValue != TYPE_INT) { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Integer());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "-")) {
            if (expr->getType()->typeValue != TYPE_INT) { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Integer());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "+.")) {
            if (expr->getType()->typeValue != TYPE_FLOAT) { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Float());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "-.")) {
            if (expr->getType()->typeValue != TYPE_FLOAT) { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Float());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "not")) {
            if (expr->getType()->typeValue != TYPE_BOOL) { 
                /* Print Error */
                std::vector<CustomType *> expectedType;
                expectedType.push_back(new Boolean());
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new Expectation(expectedType, expr->getType());
                err->printError();
            }
            this->type = expr->getType();
        }
        else { /* Left for debugging */ }
    }

private:
const char * op;
Expr *expr;
};

class IntConst : public Constant, public Pattern {
public:
    IntConst(int ic) { intConst = ic; type = new Integer(); }
    IntConst(int ic, char s) {
        intConst = (s == '+') ? ic : -ic;
        type = new Integer();
    }

    virtual void printOn(std::ostream &out) const override {
        out << intConst;
    }

    virtual void sem() override { this->type = new Integer(); }

private:
int intConst;
};

class FloatConst : public Constant, public Pattern {
public:
    FloatConst(float fc) { floatConst = fc; type = new Float(); }
    FloatConst(float fc, const char * s) {
        floatConst = ( strcmp(s, "+.") == 0 ) ? fc : -fc;
        type = new Float();
    }

    virtual void printOn(std::ostream &out) const override {
        out << floatConst;
    }

    virtual void sem() override { this->type = new Float(); }

private:
float floatConst;
};

class CharConst : public Constant, public Pattern {
public:
    CharConst(char cc): charConst(cc) { type = new Character(); }

    virtual void printOn(std::ostream &out) const override {
        out << charConst;
    }

    virtual void sem() override { this->type = new Character(); }

private:
const char charConst;
};

class StringLiteral : public Constant, public Expr {
public:
    StringLiteral(const char *sl): stringLiteral(sl) { type = new Array(new Character(), 1); }

    virtual void printOn(std::ostream &out) const override {
        out << stringLiteral;
    }

    virtual void sem() override { this->type = new Array(new Character(), 1); }

private:
const char * stringLiteral;
};

class BooleanConst : public Constant, public Pattern {
public:
    BooleanConst(bool b): boolean(b) { type = new Boolean(); }

    virtual void printOn(std::ostream &out) const override {
        (boolean) ? out << "true" : out << "false";
    }

    virtual void sem() override { this->type = new Boolean(); }

private:
const bool boolean;
};

class UnitConst : public Constant, public Expr {
public:
    UnitConst() { type = new Unit(); }

    virtual void printOn(std::ostream &out) const override {
        out << "unit";
    }

    virtual void sem() override { this->type = new Unit(); }

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

    CustomType *type;
    TypeGen *typeGen;
};

class Constr : public Expr {
public:
    Constr(std::string id, TypeGen *tg): Id(id), typeGen(tg) { call = false; }
    Constr(std::string id, Expr *e, ExprGen *eg): Id(id), expr(e), exprGen(eg) { call = true; }

    virtual void printOn(std::ostream &out) const override {
        if (expr == nullptr){
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
                    if(expr->getType()->typeValue == TYPE_UNKNOWN && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0)->typeValue != TYPE_UNKNOWN) {
                        SymbolEntry *tempParam = expr->sem_getExprObj();
                        if(tempParam->type->typeValue == TYPE_FUNC && dynamic_cast<Function*>(tempParam->type)->outputType != nullptr && dynamic_cast<Function*>(tempParam->type)->outputType->typeValue == TYPE_UNKNOWN) {
                            dynamic_cast<Function*>(tempParam->type)->outputType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                            expr->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0));
                        }
                        else {
                            tempParam->type = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                            expr->setType(tempParam->type);
                        }
                    }
                    /* type check */
                    if (expr->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0)->typeValue) { 
                        /* Print Error - type mismatch on first param */ 
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new TypeMismatch(expr->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0));
                        err->printError();
                    }
                }
                /* Type check the rest of the params */
                if (exprGen != nullptr) {
                    exprGen->sem();
                    ExprGen* tempExprGen = exprGen;
                    int i = 1;
                    while(tempExprGen != nullptr){
                        /* type inference */
                        if(tempExprGen->getExpr()->getType()->typeValue == TYPE_UNKNOWN && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue != TYPE_UNKNOWN) {
                            SymbolEntry *tempParam = tempExprGen->getExpr()->sem_getExprObj();
                            if(tempParam->type->typeValue == TYPE_FUNC && dynamic_cast<Function*>(tempParam->type)->outputType != nullptr && dynamic_cast<Function*>(tempParam->type)->outputType->typeValue == TYPE_UNKNOWN) {
                                dynamic_cast<Function*>(tempParam->type)->outputType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i);
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
                            if(tempExprGen->getExpr()->getType()->typeValue == TYPE_ID && tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_ID && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue == TYPE_CUSTOM 
                            && tempExprGen->getExpr()->sem_getExprObj()->params.front()->type->name != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->name) {
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(tempExprGen->getExpr()->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                                err->printError();
                            }
                            /* if it's a different type */
                            if((tempExprGen->getExpr()->getType()->typeValue != TYPE_ID || tempExprGen->getExpr()->sem_getExprObj()->type->typeValue != TYPE_ID || dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue != TYPE_CUSTOM)
                            && tempExprGen->getExpr()->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue) {
                                std::cout << "Line " <<__LINE__ << " -> ";
                                Error *err = new TypeMismatch(tempExprGen->getExpr()->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                                err->printError();
                            }
                        }
                        i++;
                        tempExprGen = tempExprGen->getNext();
                    }
                }
                // st.printST();
                this->type = tempEntry->type;
            }
            else { 
                this->type = new Unknown(); 
                /* Print Error - Id not exist in st */
                std::cout << "Line " <<__LINE__ << " -> ";
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
                while (tempTypeGen != nullptr){
                    if (typeGen->type->typeValue == TYPE_ID && !st.lookup(typeGen->type->name)) { 
                        /* Print Error */ 
                        // not implemented yet
                        std::cout << "Line " <<__LINE__ << " -> ";
                        Error *err = new Error(typeGen->type->name + " doesn't exist in ST");
                        err->printMessage();
                        exit(1);
                    }
                    dynamic_cast<CustomId*>(ct)->pushToParams(tempTypeGen->type);
                    tempTypeGen = tempTypeGen->typeGen;
                }
            }
            if (!st.lookup(Id, ENTRY_CONSTRUCTOR)) { st.insert(Id, ct, ENTRY_CONSTRUCTOR); }
            else { 
                /* Print Error - duplicate type color = Red | Red | Blue | Yellow */ 
                std::cout << "Line " <<__LINE__ << " -> ";
                Error *err = new DuplicateEntry(Id, false);
                err->printError();
            }
        }
    }

    virtual SymbolEntry *sem_getExprObj() override { return st.lookup(Id); }

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

    virtual void sem() override {
        /* Check if type is already in st */
        if (!st.lookup(id, ENTRY_TYPE)) { 
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
                /* Constructors of a user defined type */
                typeEntry->params.push_back(tempConstr);
                /* User defined type in a Constructor (single argument) */
                tempConstr->params.push_back(typeEntry);
                tempBarConstrGen = tempBarConstrGen->getNext();
            }
        }
        else { 
            /* Print Error - duplicate decl of type */
            std::cout << "Line " <<__LINE__ << " -> ";
            Error *err = new DuplicateEntry(id, true);
            err->printError();
        }
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

    virtual void sem() override {
        tDef->sem();
        if (tDefGen != nullptr) tDefGen->sem();
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
        for(auto constr : tempEntry->params){
            /* if its *type* params (Constr of {params}) are not empty */
            if(!dynamic_cast<CustomId*>(constr->type)->getParams().empty()){
                int i = 0;
                /* for every *type* param */
                for(auto typeParam : dynamic_cast<CustomId*>(constr->type)->getParams()){
                    /* if current *type* param is CustomId, make it point to user defined type, else it's not defined */
                    if(typeParam->typeValue == TYPE_ID){
                        SymbolEntry *typeEntry = st.lookup(typeParam->name);
                        if(typeEntry != nullptr) dynamic_cast<CustomId*>(constr->type)->replaceParam(typeEntry->type, i);
                        else { 
                            /* Print Error - unbound constructor */
                            std::cout << "Line " <<__LINE__ << " -> ";
                            Error *err = new Error("Unbound constructor");
                            err->printMessage();
                        }
                    }
                    i++;
                }
            }
        }
    }

private:
    Tdef *tDef;
    TdefGen *tDefGen;  
};

#endif