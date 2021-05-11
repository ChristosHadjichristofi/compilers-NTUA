#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>
#include "symbol.hpp"

class AST {
public:
    virtual void printOn(std::ostream &out) const = 0;
    virtual ~AST() {}
};

inline std::ostream & operator<< (std::ostream &out, const AST &ast) {
    ast.printOn(out);
    return out;
}

class Let : public AST {
public:
    Let(Def *d, DefGen *dg, bool isRec): def(d), defGen(dg), rec(isRec) {}
    
    virtual void printOn(std::ostream &out) const override {

        if (defGen == nullptr) {
            (rec) ? out << "LetRec(" << *def << ")" : out << "Let(" << *def << ")";
        }
        else {
            if (rec) out << "LetRec(" << *def << ", " << *defGen << ")" ;
            else out << "Let(" << *def << ", " << *defGen << ")";
        }
    
    }

private:
    Def *def;
    DefGen *defGen;
    bool rec;
};

class DefGen : public AST {
public:
    DefGen(Def *d, DefGen *dg): def(d), defGen(dg) {}

    virtual void printOn(std::ostream &out) const override {
        
        (defGen == nullptr) ? out << "DefGen(" << *def << ")" : out << "DefGen(" << *def << ", " << *defGen << ")"; 

    }

private:
    Def *def;
    DefGen *defGen;
};

class Def : public AST {
public:
    Def(Id *id, ParGen *pg, Expr *e, Type *t, CommaExprGen *ceg, bool isMutable): 
        id(id), parGen(pg), expr(e), type(t), commaExprGen(ceg), mut(isMutable) {}

    virtual void printOn(std::ostream &out) const override {

        if (mut) out << "MutableDef(" << *id; else out << "Def(" << *id;
        if (parGen != nullptr) out <<", " << *parGen;
        if (expr != nullptr) out <<", " << *expr;
        if (type != nullptr) out <<", " << *type;
        if (commaExprGen != nullptr) out <<", " << *commaExprGen;
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

class ParGen : public AST {
public:
    ParGen(Par *p, ParGen *pg): par(p), parGen(pg) {}

    virtual void printOn(std::ostream &out) const override {

        (parGen == nullptr) ? out << "ParGen(" << *par << ")" : out << "ParGen(" << *par << ", " << *parGen << ")";
        
    }

private:
    Par *par;
    ParGen *parGen;
};

class CommaExprGen : public AST {
public:
    CommaExprGen(Expr *e, CommaExprGen *ceg): expr(e), commaExprGen(ceg) {}

    virtual void printOn(std::ostream &out) const override {

        (commaExprGen == nullptr) ? out << "CommaExprGen(" << *expr << ")" : out << "CommaExprGen(" << *expr << ", " << *commaExprGen << ")";

    }

private:
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class TypeDef : public AST {
public:
    TypeDef(Tdef *td, TdefGen *tdg): tDef(td), tDefGen(tdg) {}

    virtual void printOn(std::ostream &out) const override {

        (tDefGen == nullptr) ? out << "TypeDef(" << *tDef << ")" : out << "TypeDef(" << *tDef << ", " << *tDefGen << ")";
        
    }

private:
    Tdef *tDef;
    TdefGen *tDefGen;  
};

class TdefGen : public AST {
public:
    TdefGen(Tdef *td, TdefGen *tdg): tDef(td), tDefGen(tdg) {}

    virtual void printOn(std::ostream &out) const override {

        (tDefGen == nullptr) ? out << "TdefGen(" << *tDef << ")" : out << "TdefGen(" << *tDef << ", " << *tDefGen << ")";
        
    }

private:
    Tdef *tDef;
    TdefGen *tDefGen;  
};

class Tdef : public AST {
public:
    Tdef(Id *id, Constr *c, BarConstrGen *bcg): id(id), constr(c), barConstrGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        (barConstrGen == nullptr) ? out << "Tdef(" << *id <<", " << *constr << ")" : out << "Tdef(" << *id <<", " << *constr << ", " << *barConstrGen << ")";
        
    }

private:
Id *id;
Constr *constr;
BarConstrGen *barConstrGen;
};

class BarConstrGen : public AST {
public:
    BarConstrGen(Constr *c, BarConstrGen *bcg): constr(c), barConstrGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        (barConstrGen == nullptr) ? out << "BarConstrGen(" << *constr << ")" : out << "BarConstrGen(" << *constr << ", " << *barConstrGen << ")";
        
    }

private:
Constr *constr;
BarConstrGen *barConstrGen;
};

class Constr : public AST {
public:
    Constr(std::string id, TypeGen *tg): Id(id), typeGen(tg) {}
    Constr(std::string id, Expr *e, ExprGen *eg): Id(id), expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {
        if (expr == nullptr){
            (typeGen == nullptr) ? out << "Constr(" << Id << ")" : out << "Constr(" << Id << ", " << *typeGen << ")";
        }
        else {
            (exprGen == nullptr) ? out << "Id(" << *expr << ")" : out << "Id(" << *expr << ", " << *exprGen << ")";  
        }
    }

private:
std::string Id;
TypeGen *typeGen;
Expr *expr;
ExprGen *exprGen;
};

class TypeGen : public AST {
public:
    TypeGen(Type *t, TypeGen *tg): type(t), typeGen(tg) {}

    virtual void printOn(std::ostream &out) const override {

        (typeGen == nullptr) ? out << "TypeGen(" << *type << ")" : out << "TypeGen(" << *type << ", " << *typeGen << ")";
        
    }

private:
Type *type;
TypeGen *typeGen;
};

class Par : public AST {
public:
    Par(std::string id, Type* t): id(id), type(t) {}

    virtual void printOn(std::ostream &out) const override {

        (type == nullptr) ? out << "Par(" << id << ")" : out << "Par(" << id << ", " << *type << ")" ;
        
    }

private:
std::string id;
Type *type;
};

class Constant : public AST {};

class IntConst : public Constant {
public:
    IntConst(int ic, char s) {
        intConst = (s == '+') ? ic : -ic;
    }

private:
int intConst;
};

class FloatConst : public Constant {
public:
    FloatConst(float fc, std::string s) {
        floatConst = (s == "+.") ? fc : -fc;
    }

private:
float floatConst;
};

class CharConst : public Constant {
public:
    CharConst(char cc): charConst(cc) {}

private:
const char charConst;
};

class StringLiteral : public Constant {
public:
    StringLiteral(std::string sl): stringLiteral(sl) {}

private:
const std::string stringLiteral;
};

class BooleanConst : public Constant {
public:
    BooleanConst(bool b): boolean(b) {}

private:
const bool boolean;
};

class Id : public AST {
public:
    Id(std::string n): name(n) {}
    Id(std::string n, Expr *e, ExprGen *eg): name(n), expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {
        (exprGen == nullptr) ? out << "Id(" << name <<", " << *expr <<")" : out << "Id(" << name <<", " << *expr <<", " << *exprGen <<")";
    }

private:
std::string name;
Expr *expr;
ExprGen *exprGen;
};

class Expr : public AST {};

class UnOp : public Expr {
public:
    UnOp(std::string op, Expr *e): op(op), expr(e)  {}

    virtual void printOn(std::ostream &out) const override {
        out << "UnOp(" << op <<", " << *expr <<")";
    }

private:
std::string op;
Expr *expr;
};

class BinOp : public Expr {
public:
    BinOp(Expr *e1, std::string op, Expr *e2): expr1(e1), op(op), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {
        out << "BinOp(" << *expr1 <<", " << op <<", " << *expr2 <<")";
    }

private:
std::string op;
Expr *expr1;
Expr *expr2;
};

class Dim : public Expr {
public:
    Dim(Id *id, int ic): id(id), intconst(ic) {};

    virtual void printOn(std::ostream &out) const override {
        (intconst == NULL) ? out << "Dim(" << *id << ")" : out << "Dim(" << *id <<", " << intconst <<")";
    }

private:
Id *id;
int intconst;
};

class ArrayItem : public Expr {
public:
    ArrayItem(Id *id, Expr *e, CommaExprGen *ceg): id(id), expr(e), commaExprGen(ceg) {}

    virtual void printOn(std::ostream &out) const override {
        (commaExprGen == nullptr) ? out << "ArrayItem(" << *id << "[" << *expr << "])" : out <<"ArrayItem(" << *id << "[" << *expr <<", " << *commaExprGen << "])";
    }

protected:
Id *id;
Expr *expr;
CommaExprGen *commaExprGen;
};

class New : public Expr {
public:
    New(Type *t): type(t) {}

    virtual void printOn(std::ostream &out) {
        out << "New(" << *type << ")";
    }

private:
Type *type;
};

class Delete : public Expr {
public:
    Delete(Expr *e): expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "Delete(" << *expr <<")";
    }

private:
Expr *expr;
};

class LetIn : public Expr {
public:
    LetIn(Let* l, Expr *e): let(l), expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "LetIn(" << *let <<", " << *expr <<")";
    }

private:
Let *let;
Expr *expr;
};

class Begin : public Expr {
public:
    Begin(Expr *e): expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        out << "Begin(" << *expr <<")";
    }

private:
Expr *expr;
};

class If : public Expr {
public:
    If(Expr *c, Expr *e1, Expr *e2): condition(c), expr1(e1), expr2(e2) {}

    virtual void printOn(std::ostream &out) const override {
        
        (expr2 == nullptr) ? out << "If(" << *condition <<", " << *expr1 <<")" : out << "If(" << *condition <<", " << *expr1 <<", " << *expr2 <<")";
    }

private:
Expr *condition;
Expr *expr1;
Expr *expr2;
};

class While : public Expr {
public:
    While(Expr *lc, Expr *e): loopCondition(lc), expr(e) {}

    virtual void printOn(std::ostream &out) const override {
        
        out << "While(" << *loopCondition <<", " << *expr <<")";
    }

private:
Expr *loopCondition;
Expr *expr;
};

class For : public Expr {
public:
    For(Id *id, Expr *s, Expr *end, Expr *e, bool isAscending):
        id(id), start(s), end(end), expr(e), ascending(isAscending) {}

    virtual void printOn(std::ostream &out) const override {
        
        out << "For(" << *id <<", " << *start <<", " << *end <<", " << *expr <<", " << ascending <<")";
    }

private:
Id *id;
Expr *start;
Expr *end;
Expr *expr;
bool ascending;
};

class Match : public Expr {
public:
    Match(Expr *e, Clause *c, BarClauseGen *bcg): expr(e), clause(c), barClauseGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Match(" << *expr << ", " << *clause << ", " << *barClauseGen <<")";

    }

private:
Expr *expr;
Clause *clause;
BarClauseGen *barClauseGen;
};

class ExprGen : public AST {
public:
    ExprGen(Expr *e, ExprGen *eg): expr(e), exprGen(eg) {}

    virtual void printOn(std::ostream &out) const override {

        (exprGen == nullptr) ? out << "ExprGen(" << *expr <<")" : out << "ExprGen(" << *expr << ", " << *exprGen <<")";

    }

private:
Expr *expr;
ExprGen *exprGen;
};

class BarClauseGen : public AST {
public:
    BarClauseGen(Clause *c, BarClauseGen *bcg): clause(c), barClauseGen(bcg) {}

    virtual void printOn(std::ostream &out) const override {

        (barClauseGen == nullptr) ? out << "BarClauseGen(" << *clause <<")" : out << "BarClauseGen(" << *clause << ", " << *barClauseGen <<")";

    }

private:
Clause *clause;
BarClauseGen *barClauseGen;
};

class Clause : public AST {
public:
    Clause(Pattern *p, Expr *e): pattern(p), expr(e) {}

    virtual void printOn(std::ostream &out) const override {

        out << "Clause(" << *pattern << ", " << *expr <<")";

    }

private:
Pattern *pattern;
Expr *expr;
};

class Pattern : public AST {};

class PatternGen : public AST {
public:
    PatternGen(Pattern *p, PatternGen *pg ): pattern(p), patternGen(pg) {}

    virtual void printOn(std::ostream &out) const override {
        (patternGen == nullptr) ? out << "PatternGen(" << *pattern << ")" : out << "PatternGen(" << *pattern << ", " << *patternGen << ")";
    }

private:
Pattern *pattern;
PatternGen *patternGen;
};

class PatternConstr : public Pattern {
public:
    PatternConstr(std::string id, PatternGen *pg): Id(id), patternGen(pg) {}

    virtual void printOn(std::ostream &out) const override {
        (patternGen == nullptr) ? out << "Id(" << Id << ")" : out << "Id(" << Id << ", " << *patternGen << ")";
    }

protected:
std::string Id;
PatternGen *patternGen;
};

#endif