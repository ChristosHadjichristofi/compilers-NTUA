#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include <string>
#include "types.hpp"

class AST {
public:
    virtual void printOn(std::ostream &out) const = 0;
    virtual ~AST() {}
};

inline std::ostream & operator<< (std::ostream &out, const AST &ast) {
    ast.printOn(out);
    return out;
}

class Constant : public AST {};
class Expr : public AST {};
class Pattern : public AST {};

class ExprGen : public AST {
public:
    ExprGen(Expr *e, ExprGen *eg): expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {

        if (exprGen == nullptr) {
            out << "ExprGen("; expr->printOn(out); out <<")";
        } 
        else { 
            out << "ExprGen("; expr->printOn(out); out << ", "; exprGen->printOn(out); out <<")"; 
        }

    }

private:
Expr *expr;
ExprGen *exprGen;
};


class Id : public AST {
public:
    Id(std::string n): name(n) {}
    Id(std::string n, Expr *e, ExprGen *eg): name(n), expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {
        if (exprGen == nullptr) {
            out << "Id(" << name <<", "; expr->printOn(out); out <<")";
        }
        else { 
            out << "Id(" << name <<", "; expr->printOn(out); out <<", "; exprGen->printOn(out); out <<")";
        }
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

private:
Pattern *pattern;
PatternGen *patternGen;
};

class PatternConstr : public Pattern {
public:
    PatternConstr(std::string id, PatternGen *pg): Id(id), patternGen(pg) {}

    virtual void printOn(std::ostream &out) const override {
        if (patternGen == nullptr) {
            out << "Id(" << Id << ")";
        }
        else {
            out << "Id(" << Id << ", "; patternGen->printOn(out); out << ")";
        }
    }

protected:
std::string Id;
PatternGen *patternGen;
};

class Clause : public AST {
public:
    Clause(Pattern *p, Expr *e): pattern(p), expr(e) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Clause("; pattern->printOn(out); out << ", "; expr->printOn(out); out <<")";

    }

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

private:
Clause *clause;
BarClauseGen *barClauseGen;
};

class Match : public Expr {
public:
    Match(Expr *e, Clause *c, BarClauseGen *bcg): expr(e), clause(c), barClauseGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Match("; expr->printOn(out); out << ", "; clause->printOn(out); out << ", "; barClauseGen->printOn(out); out <<")";

    }

private:
Expr *expr;
Clause *clause;
BarClauseGen *barClauseGen;
};

class For : public Expr {
public:
    For(Id *id, Expr *s, Expr *end, Expr *e, bool isAscending):
        id(id), start(s), end(end), expr(e), ascending(isAscending) {}

    virtual void printOn(std::ostream &out) const override {
        
        out << "For("; id->printOn(out); out <<", "; start->printOn(out); out <<", "; end->printOn(out); out <<", "; expr->printOn(out); out <<", " << ascending <<")";
    }

private:
Id *id;
Expr *start;
Expr *end;
Expr *expr;
bool ascending;
};

class While : public Expr {
public:
    While(Expr *lc, Expr *e): loopCondition(lc), expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        
        out << "While("; loopCondition->printOn(out); out <<", "; expr->printOn(out); out <<")";
    }

private:
Expr *loopCondition;
Expr *expr;
};

class If : public Expr {
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

private:
Expr *condition;
Expr *expr1;
Expr *expr2;
};

class Begin : public Expr {
public:
    Begin(Expr *e): expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "Begin("; expr->printOn(out); out <<")";
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

private:
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class Par : public AST {
public:
    Par(std::string id, Type* t): id(id), type(t) {}

    virtual void printOn(std::ostream &out) const override {

        if (type == nullptr) {
            out << "Par(" << id << ")";
        } 
        else {
            out << "Par(" << id << ", "; type->printOn(out); out << ")" ;
        }
        
    }

private:
std::string id;
Type *type;
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

private:
    Par *par;
    ParGen *parGen;
};

class Def : public AST {
public:
    Def(Id *id, ParGen *pg, Expr *e, Type *t, CommaExprGen *ceg, bool isMutable): 
        id(id), parGen(pg), expr(e), type(t), commaExprGen(ceg), mut(isMutable) {}

    virtual void printOn(std::ostream &out) const override {

        if (mut){ out << "MutableDef("; id->printOn(out); } else { out << "Def("; id->printOn(out); };
        if (parGen != nullptr) { out <<", "; parGen->printOn(out); }
        if (expr != nullptr) { out <<", "; expr->printOn(out); }
        if (type != nullptr) { out <<", "; type->printOn(out); }
        if (commaExprGen != nullptr) { out <<", "; commaExprGen->printOn(out); }
        out << ")";
    }

private:
    Id *id;
    ParGen *parGen;
    Expr *expr;
    Type *type;
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

private:
    Def *def;
    DefGen *defGen;
};

class Let : public AST {
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

private:
    Def *def;
    DefGen *defGen;
    bool rec;
};

class LetIn : public Expr {
public:
    LetIn(Let* l, Expr *e): let(l), expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "LetIn("; let->printOn(out); out <<", "; expr->printOn(out); out <<")";
    }

private:
Let *let;
Expr *expr;
};

class Delete : public Expr {
public:
    Delete(Expr *e): expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "Delete("; expr->printOn(out); out <<")";
    }

private:
Expr *expr;
};

class New : public Expr {
public:
    New(Type *t): type(t) {}

    virtual void printOn(std::ostream &out) const override {
        out << "New("; type->printOn(out); out << ")";
    }

private:
Type *type;
};

class ArrayItem : public Expr {
public:
    ArrayItem(Id *id, Expr *e, CommaExprGen *ceg): id(id), expr(e), commaExprGen(ceg) {}

    virtual void printOn(std::ostream &out) const override {

        if (commaExprGen == nullptr) {
            out << "ArrayItem("; id->printOn(out); out << "["; expr->printOn(out); out << "])";
        }
        else {
            out <<"ArrayItem("; id->printOn(out); out << "["; expr->printOn(out); out <<", "; commaExprGen->printOn(out); out << "])";
        }
    
    }

protected:
Id *id;
Expr *expr;
CommaExprGen *commaExprGen;
};

class Dim : public Expr {
public:
    Dim(Id *id, int ic): id(id), intconst(ic) {};

    virtual void printOn(std::ostream &out) const override {

        if (intconst == NULL) {
            out << "Dim("; id->printOn(out); out << ")";
        }
        else {
            out << "Dim("; id->printOn(out); out <<", " << intconst <<")";
        }
    
    }

private:
Id *id;
int intconst;
};

class BinOp : public Expr {
public:
    BinOp(Expr *e1, std::string op, Expr *e2): expr1(e1), op(op), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {
        out << "BinOp("; expr1->printOn(out); out <<", " << op <<", "; expr2->printOn(out); out <<")";
    }

private:
std::string op;
Expr *expr1;
Expr *expr2;
};

class UnOp : public Expr {
public:
    UnOp(std::string op, Expr *e): op(op), expr(e)  {}

    virtual void printOn(std::ostream &out) const override {
        out << "UnOp(" << op <<", "; expr->printOn(out); out <<")";
    }

private:
std::string op;
Expr *expr;
};

class IntConst : public Constant, public Pattern {
public:
    IntConst(int ic, char s) {
        intConst = (s == '+') ? ic : -ic;
    }

    virtual void printOn(std::ostream &out) const override {
        out << intConst;
    }

private:
int intConst;
};

class FloatConst : public Constant, public Pattern {
public:
    FloatConst(float fc, std::string s) {
        floatConst = (s == "+.") ? fc : -fc;
    }

    virtual void printOn(std::ostream &out) const override {
        out << floatConst;
    }

private:
float floatConst;
};

class CharConst : public Constant, public Expr, public Pattern {
public:
    CharConst(char cc): charConst(cc) {}

    virtual void printOn(std::ostream &out) const override {
        out << charConst;
    }

private:
const char charConst;
};

class StringLiteral : public Constant {
public:
    StringLiteral(std::string sl): stringLiteral(sl) {}

    virtual void printOn(std::ostream &out) const override {
        out << stringLiteral;
    }

private:
const std::string stringLiteral;
};

class BooleanConst : public Constant, public Expr, public Pattern {
public:
    BooleanConst(bool b): boolean(b) {}

    virtual void printOn(std::ostream &out) const override {
        (boolean) ? out << "true" : out << "false";
    }

private:
const bool boolean;
};

class UnitConst : public Constant, public Expr {
public:
    UnitConst() {}

    virtual void printOn(std::ostream &out) const override {
        out << "unit";
    }

};

class TypeGen : public AST {
public:
    TypeGen(Type *t, TypeGen *tg): type(t), typeGen(tg) {}

    virtual void printOn(std::ostream &out) const override {

        if (typeGen == nullptr) {
            out << "TypeGen("; type->printOn(out); out << ")";
        }
        else {
            out << "TypeGen("; typeGen->printOn(out); out << ", "; type->printOn(out); out << ")";
        }
        
    }

private:
Type *type;
TypeGen *typeGen;
};

class Constr : public AST {
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
    Tdef(Id *id, Constr *c, BarConstrGen *bcg): id(id), constr(c), barConstrGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        if (barConstrGen == nullptr) {
            out << "Tdef("; id->printOn(out); out <<", "; constr->printOn(out); out << ")";
        }
        else {
            out << "Tdef("; id->printOn(out); out <<", "; constr->printOn(out); out << ", "; barConstrGen->printOn(out); out << ")";
        }
        
    }

private:
Id *id;
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

class TypeDef : public AST {
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