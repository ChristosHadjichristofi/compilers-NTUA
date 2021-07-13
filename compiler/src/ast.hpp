#ifndef __AST_HPP__
#define __AST_HPP__

#include <cstring>
#include <iostream>
#include <vector>
#include "types.hpp"
#include "AST.hpp"
#include "symbol.hpp"

inline std::ostream & operator<< (std::ostream &out, const AST &ast) {
    ast.printOn(out);
    return out;
}

class Constant : public AST {};
class Pattern : public AST {
public:
    virtual SymbolEntry *sem_getExprObj() { /* Print Error */ }
};

class Expr : public AST {
public:

    // void typeCheck(Types t) {
    //     if (type != t) {
    //         // type mismatch
    //     }
    // }
    
    /* Needed for Class Clause -> call from Match */
    virtual Expr *sem_getClauseExpr(SymbolEntry *se) {}
    /* Needed for Class Id | Constr | PatternConstr -> call from Match */
    virtual SymbolEntry *sem_getExprObj() { /* Print Error */ }

    CustomType *getType() {
        return type;
    }

protected:
CustomType *type;
};

class Block : public AST {
public:
    Block(): block() {}

    void appendBlock(Block *b){
        block.push_back(b);
    }

    virtual void printOn(std::ostream &out) const override {
        
        out << "Block(";
        bool first = true;
        for(Block *b : block) {
            if (!first) out << ", ";
            first = false;
            b->printOn(out);
        }
        out<< ")";
    }

    virtual void sem() override {
        st.openScope();
        for(Block* b : block) b->sem();
        st.closeScope();
    }

protected:
std::vector<Block*> block;
};

class ExprGen : public AST {
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

    virtual void sem() override {
        expr->sem();
        if (exprGen != nullptr) exprGen->sem();
    }

private:
Expr *expr;
ExprGen *exprGen;
};

class Id : public Expr, public Pattern {
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

    virtual SymbolEntry *sem_getExprObj() override {
        return st.lookup(name);
    }

    virtual void sem() override {
        /* lookup for variable, if exists [might need to be moved down] */
        if (expr == nullptr && exprGen == nullptr) {
            SymbolEntry *tempEntry = st.lookup(name);
            if (!tempEntry) this->type = tempEntry->type;
            else { /* Print Error First Occurance*/ }
        }
        /* lookup for first param of a function */
        if (expr != nullptr) expr->sem();
        /* lookup for the rest params of a function, if they exist */
        if (exprGen != nullptr) exprGen->sem();
    }

private:
std::string name;
Expr *expr;
ExprGen *exprGen;
};


class PatternGen : public AST {
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

    virtual void sem() override {
        pattern->sem();
        if (patternGen != nullptr) 
            patternGen->sem();
    }

private:
Pattern *pattern;
PatternGen *patternGen;
};

class PatternConstr : public Pattern {
public:
    PatternConstr(char * id, PatternGen *pg): Id(id), patternGen(pg) {}

    virtual void printOn(std::ostream &out) const override {
        if (patternGen == nullptr) {
            out << "Id(" << Id << ")";
        }
        else {
            out << "Id(" << Id << ", "; patternGen->printOn(out); out << ")";
        }
    }

protected:
char * Id;
PatternGen *patternGen;
};

class Clause : public Expr {
public:
    Clause(Pattern *p, Expr *e): pattern(p), expr(e) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Clause("; pattern->printOn(out); out << ", "; expr->printOn(out); out <<")";

    }

    /* this symbol entry has name attribute and val array with X positions of types given -> create by Constr */
    virtual Expr *sem_getClauseExpr(SymbolEntry *se) override {

        // compare se.name to p.name if true then compare se.val[] to p.val[] if true expr->sem()
        if (se->id == pattern->sem_getExprObj()->id) {
            // value check if true 
            return expr;
        }
        return nullptr;
    }

    // virtual void sem() override {
    //     pattern->sem();
    //     expr->sem();
    // }

private:
Pattern *pattern;
Expr *expr;
};

class BarClauseGen : public AST {
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

    virtual Expr *sem(SymbolEntry *se) {
        /* if reached here, more clauses exist */

        /* pass the tempEntry to clause in order to compare it with every pattern */
        /* in case that matched the expression will be returned else nullptr will be returned */
        Expr *returnedExpr = clause->sem_getClauseExpr(se);
        if (!returnedExpr) { return returnedExpr; }
        /* in case of not matched with the first clause and more clauses exist, make the check */
        if (barClauseGen != nullptr) barClauseGen->sem(se);
        else { /* throw no match Error */ }
    }

    // virtual void sem() override {
    //     clause->sem();
    //     if(barClauseGen != nullptr)
    //         barClauseGen->sem();
    // }

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
        /* Need symbol entry in order to compare it with all patterns */ 
        SymbolEntry *tempEntry = expr->sem_getExprObj();
        /* pass the tempEntry to clause in order to compare it with every pattern */
        /* in case that matched the expression will be returned else nullptr will be returned */
        Expr *returnedExpr = clause->sem_getClauseExpr(tempEntry);
        if (!returnedExpr) { returnedExpr->sem(); return; }
        /* in case of not matched with the first clause and more clauses exist, make the check */
        if (barClauseGen != nullptr) returnedExpr = barClauseGen->sem(tempEntry);
        if (!returnedExpr) returnedExpr->sem();
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
        /* goes to BinOp and needs to openscope and save the variable with its value */
        start->sem();
        if (start->getType()->typeValue != TYPE_INT) { /* Print Error */ }
        /* goes to BinOp, UnOp, Id, Dim */
        end->sem();
        if (end->getType()->typeValue != TYPE_INT) { /* Print Error */ }
        /* if everything ok then proceed */
        expr->sem();
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
        /* goes to BooleanConst, If, BinOp, LetIn, Begin(?) */
        loopCondition->sem();
        if (loopCondition->getType()->typeValue != TYPE_BOOL) { /* Print Error */ }
        expr->sem();
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
        if (condition->getType()->typeValue != TYPE_BOOL) { /* print Error */ }
        expr1->sem();
        if(expr2 != nullptr) {
            expr2->sem();
            /* expr1 and expr2 must be of same type */
            if (expr1->getType()->typeValue != expr2->getType()->typeValue) { /* print Error */ }
        }
        this->type = expr1->getType();
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
    Par(char * id, CustomType* t): id(id), type(t) {}

    virtual void printOn(std::ostream &out) const override {

        if (type == nullptr) {
            out << "Par(" << id << ")";
        } 
        else {
            out << "Par(" << id << ", "; type->printOn(out); out << ")" ;
        }
        
    }

    virtual void sem() override {
        if(type != nullptr) st.insert(id, type);
        else st.insert(id, new Unknown());
    }

private:
char * id;
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

    virtual void sem() override {
        SymbolEntry *tempEntry = st.getLastEntry();
        if (parGen != nullptr) dynamic_cast<Function*>(tempEntry->type)->outputType = new Function(new Unknown(), new Unknown());
        par->sem();
        dynamic_cast<Function*>(tempEntry->type)->inputType = st.getLastEntry()->type;
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
                if (type != nullptr) st.insert(id, new Reference(type));
                /* variable's type is unknown */
                else st.insert(id, new Reference(new Unknown()));
            }
            /* array */
            else {
                expr->sem();
                if (expr->getType()->typeValue != TYPE_INT) { /* Throw Error */ }
                if (commaExprGen != nullptr) commaExprGen->sem();
                
                /* get dimensions by iterating commaExprGen "list" */
                int dimensions = 1;
                CommaExprGen *tempExpr = commaExprGen;
                while (tempExpr->getNext() != nullptr) {
                    dimensions++;
                    tempExpr = tempExpr->getNext();
                }
                
                /* array's type is given */
                if (type != nullptr) st.insert(id, new Reference(new Array(type, dimensions)));
                /* array's type is unknown */
                else st.insert(id, new Reference(new Array(new Unknown(), dimensions)));
            }
        }
        else {
            /* if def is a non mutable variable - constant */
            if (parGen == nullptr) {
                expr->sem();
                /* not null type */
                if (type != nullptr) {
                    /* check if type given is same as expression's */
                    if (type->typeValue == expr->getType()->typeValue) st.insert(id, type);
                    /* not equal => Error */
                    else { /* Throw Error */ }
                }
                /* null type means that type is equal to expression's */
                else st.insert(id, expr->getType());
            }
            /* if def is a function */
            else {
                /* if type of function (return type) is given */
                if (type != nullptr) st.insert(id, new Function(new Unknown(), type));
                /* if type of function (return type) is not given */
                else st.insert(id, new Function(new Unknown(), new Unknown()));
                st.openScope();
                parGen->sem();
                expr->sem();
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

    virtual void sem() override {}

private:
Expr *expr;
};

class New : public Expr {
public:
    New(CustomType *t): type(t) {}

    virtual void printOn(std::ostream &out) const override {
        out << "New("; type->printOn(out); out << ")";
    }

    virtual void sem() override {}

private:
CustomType *type;
};

class ArrayItem : public Expr {
public:
    ArrayItem(char *id, Expr *e, CommaExprGen *ceg): id(id), expr(e), commaExprGen(ceg) {}

    virtual void printOn(std::ostream &out) const override {

        if (commaExprGen == nullptr) {
            out << "ArrayItem("<< id << "["; expr->printOn(out); out << "])";
        }
        else {
            out <<"ArrayItem("<< id << "["; expr->printOn(out); out <<", "; commaExprGen->printOn(out); out << "])";
        }
    
    }

    virtual void sem() override {

    }

protected:
char *id;
Expr *expr;
CommaExprGen *commaExprGen;
};

class Dim : public Expr {
public:
    Dim(char *id): id(id) { initialized = false; }
    Dim(char *id, int ic): id(id), intconst(ic) { initialized = true; }
    
    virtual void printOn(std::ostream &out) const override {

        if (!initialized) {
            out << "Dim("<< id << ")";
        }
        else {
            out << "Dim("<< id <<", " << intconst <<")";
        }
    
    }

private:
char *id;
int intconst;
bool initialized;
};

class BinOp : public Expr {
public:
    BinOp(Expr *e1, const char * op, Expr *e2): expr1(e1), op(op), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {
        out << "BinOp("; expr1->printOn(out); out <<", " << op <<", "; expr2->printOn(out); out <<")";
    }

    virtual std::pair<CustomType *, int> getRefFinalType(CustomType *ct) {

        int levels = 1;
        CustomType *obj = ct->ofType;

        while (obj->typeValue != TYPE_REF) {
            levels++;
            obj = obj->ofType;
        }
        
        return std::make_pair(obj, levels);

    } 

    virtual void sem() override {

        expr1->sem();
        expr2->sem();

        if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "mod")) {
            this->type = new Integer();
            if (expr1->getType()->typeValue == TYPE_INT && expr2->getType()->typeValue == TYPE_INT) {}
            else { /* Print Error */ }
        }
        else if (!strcmp(op, "+.") || !strcmp(op, "-.") || !strcmp(op, "*.") || !strcmp(op, "/.") || !strcmp(op, "**")) {
            this->type = new Float();
            if (expr1->getType()->typeValue == TYPE_FLOAT && expr2->getType()->typeValue == TYPE_FLOAT) {}
            else { /* Print Error */ }
        }
        else if (!strcmp(op, "=") || !strcmp(op, "<>")) {            
            /* the result will always be boolean */
            this->type = new Boolean();

            if (expr1->getType()->typeValue == expr2->getType()->typeValue 
             && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {
                // compare values
            }
            else { /* Print Error */ }
        }
        else if (!strcmp(op, "==") || !strcmp(op, "!=")) {
            this->type = new Boolean();
            if (expr1->getType() == expr2->getType() 
             && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {
                // value check
            }
            else { /* Print Error */ }
        } 
        else if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, ">=") || !strcmp(op, "<=")) {
            this->type = new Boolean();
            if (expr1->getType()->typeValue == expr2->getType()->typeValue 
             && (expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT 
             || expr1->getType()->typeValue == TYPE_CHAR)) {
                // value check
            }
            else { /* Print Error */ }
        }
        else if (!strcmp(op, "&&") || !strcmp(op, "||")) {
            this->type = new Boolean();
            if (expr1->getType()->typeValue == expr2->getType()->typeValue 
             && (expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT 
             || expr1->getType()->typeValue == TYPE_CHAR)) {
                // value check
            }
            else { /* Print Error */ }
        }
        else if (!strcmp(op, ";")) {
            this->type = expr2->getType();
        }
        else if (!strcmp(op, ":=")) {
            this->type = new Unit();
            SymbolEntry *tempEntry = expr1->sem_getExprObj();
            if (!tempEntry) {
                // if expr1 = Ref(Unknown) then replace Unknown with expr2 type
                if (expr1->getType()->ofType->typeValue == TYPE_UNKNOWN) expr1->getType()->ofType = expr2->getType();
                // expr1 already has a ref type so need to compare type with expr2 type
                else {
                    std::pair <CustomType *, int> pairExpr1, pairExpr2;

                    if (expr1->getType()->typeValue == TYPE_REF) pairExpr1 = getRefFinalType(expr1->getType()->ofType);
                    if (expr2->getType()->typeValue == TYPE_REF) pairExpr2 = getRefFinalType(expr2->getType());
                    
                    if (expr1->getType()->ofType->typeValue == expr2->getType()->typeValue) {

                        if (expr2->getType()->typeValue == TYPE_REF) {
                            if (pairExpr1.first->typeValue == pairExpr2.first->typeValue && pairExpr1.second == pairExpr2.second) {
                                // if matches then value should be changed
                            }
                            else { /* Print Error */ }
                        }
                        else { /* if matches then value should be changed */ }

                    }
                    else { /* Print Error - type mismatch */ }
                }
            }
            else { /* Print Error - var not exist (first occurance) */ }
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

    virtual void sem() override {
        expr->sem();
        if (!strcmp(op, "!")) {
            /* If expr is Ref(type), make Dereference (convert Ref(type) to type) */
            if (expr->getType()->typeValue == TYPE_REF) {
                this->type = expr->getType()->ofType;
            }
            /* If expr is not Ref(type) - reached the end of References - make an Invalid type Type(Ref(nullptr)) */
            else {
                this->type = expr->getType();
                this->type->ofType = new Reference(this->type->ofType);
            }

            SymbolEntry *tempEntry = expr->sem_getExprObj();
            if (!tempEntry) {
                /* if array */
                if (tempEntry->type->typeValue == TYPE_ARRAY) {

                }
                /* if variable */
                else {

                }
            }
            else { /* Print Error */ }

        }
        else if (!strcmp(op, "+")) {
            if (expr->getType()->typeValue != TYPE_INT) { /* Print Error */ return; }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "-")) {
            if (expr->getType()->typeValue != TYPE_INT) { /* Print Error */ return; }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "+.")) {
            if (expr->getType()->typeValue != TYPE_FLOAT) { /* Print Error */ return; }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "-.")) {
            if (expr->getType()->typeValue != TYPE_FLOAT) { /* Print Error */ return; }
            this->type = expr->getType();
        }
        else if (!strcmp(op, "not")) {
            if (expr->getType()->typeValue != TYPE_BOOL) { /* Print Error */ return; }
            this->type = expr->getType();
        }
        else { /* Left for debugging */ }
    }

private:
const char * op;
Expr *expr;
};

class IntConst : public Constant, public Expr, public Pattern {
public:
    IntConst(int ic) { intConst = ic; type = new Integer(); }
    IntConst(int ic, char s) {
        intConst = (s == '+') ? ic : -ic;
        type = new Integer();
    }

    virtual void printOn(std::ostream &out) const override {
        out << intConst;
    }

private:
int intConst;
};

class FloatConst : public Constant, public Expr, public Pattern {
public:
    FloatConst(float fc) { floatConst = fc; type = new Float(); }
    FloatConst(float fc, const char * s) {
        floatConst = ( strcmp(s, "+.") == 0 ) ? fc : -fc;
        type = new Float();
    }

    virtual void printOn(std::ostream &out) const override {
        out << floatConst;
    }

private:
float floatConst;
};

class CharConst : public Constant, public Expr, public Pattern {
public:
    CharConst(char cc): charConst(cc) { type = new Character(); }

    virtual void printOn(std::ostream &out) const override {
        out << charConst;
    }

private:
const char charConst;
};

class StringLiteral : public Constant, public Expr {
public:
    StringLiteral(const char *sl): stringLiteral(sl) { type = new String(); }

    virtual void printOn(std::ostream &out) const override {
        out << stringLiteral;
    }

private:
const char * stringLiteral;
};

class BooleanConst : public Constant, public Expr, public Pattern {
public:
    BooleanConst(bool b): boolean(b) { type = new Boolean(); }

    virtual void printOn(std::ostream &out) const override {
        (boolean) ? out << "true" : out << "false";
    }

private:
const bool boolean;
};

class UnitConst : public Constant, public Expr {
public:
    UnitConst() { type = new Unit(); }

    virtual void printOn(std::ostream &out) const override {
        out << "unit";
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

private:
CustomType *type;
TypeGen *typeGen;
};

class Constr : public Expr {
public:
    Constr(std::string id, TypeGen *tg): Id(id), typeGen(tg) {}
    Constr(std::string id, Expr *e, ExprGen *eg): Id(id), expr(e), exprGen(eg) {}

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
                out << "Id("; expr->printOn(out); out << ")";
            }
            else {
                out << "Id("; expr->printOn(out); out << ", "; exprGen->printOn(out); out << ")";  
            }
        }
    }

    virtual SymbolEntry *sem_getExprObj() {
        return st.lookup(Id);
    }

private:
std::string Id;
TypeGen *typeGen;
Expr *expr;
ExprGen *exprGen;
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

private:
Constr *constr;
BarConstrGen *barConstrGen;
};

class Tdef : public AST {
public:
    Tdef(char *id, Constr *c, BarConstrGen *bcg): id(id), constr(c), barConstrGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        if (barConstrGen == nullptr) {
            out << "Tdef("<< id <<", "; constr->printOn(out); out << ")";
        }
        else {
            out << "Tdef("<< id <<", "; constr->printOn(out); out << ", "; barConstrGen->printOn(out); out << ")";
        }
        
    }

private:
char *id;
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

private:
    Tdef *tDef;
    TdefGen *tDefGen;  
};

#endif