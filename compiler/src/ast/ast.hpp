#ifndef __AST_HPP__
#define __AST_HPP__

#include <cstring>
#include <iostream>
#include <string>
#include "AST.hpp"
#include "../types/types.hpp"
#include "../error/error.hpp"
#include "../symbol/symbol.hpp"

extern int comment_depth;
extern std::vector<SymbolEntry *> recFunctions;

inline std::ostream & operator<< (std::ostream &out, const AST &ast) {
    ast.printOn(out);
    return out;
}

class Constant : public AST {};

class Expr : public AST {
public:
    virtual SymbolEntry *sem_getExprObj();
    virtual std::string getName();
    CustomType *getType();
    void setType(CustomType *t);
    virtual std::pair<CustomType *, int> getRefFinalType(CustomType *ct) const;
    virtual std::pair<CustomType *, int> getFnFinalType(CustomType *ct) const;
    virtual llvm::Value* compile() override;
    void setRecInfo(bool irf, std::string rfn);
    bool isRec();
    std::string getRecFuncName();

protected:
    CustomType *type = nullptr;
    bool isRecFunc = false;
    std::string recFuncName = "";
};

class Pattern : public Expr {
public:
    void setMatchExprV(llvm::Value *v);
    void setNextClauseBlock(llvm::BasicBlock *bb);
    virtual std::set<std::string> preCompile();
    llvm::Value *matchExprV;
    llvm::BasicBlock *nextClauseBlock;
};

class Block : public AST {
public:
    Block();
    void appendBlock(Block *b);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

protected:
    std::vector<Block*> block;
};

class ExprGen : public Expr {
public:
    ExprGen(Expr *e, ExprGen *eg);
    virtual void printOn(std::ostream &out) const override;
    Expr *getExpr();
    ExprGen *getNext();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *expr;
    ExprGen *exprGen;
};

class Id : public Expr {
public:
    Id(std::string n);
    Id(std::string n, Expr *e, ExprGen *eg);
    virtual void printOn(std::ostream &out) const override;
    std::string getName() override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    std::string name;
    Expr *expr = nullptr;
    ExprGen *exprGen = nullptr;
};

class PatternId : public Pattern {
public:
    PatternId(std::string id);
    virtual void printOn(std::ostream &out) const override;
    std::string getName() override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

protected:
    std::string name;
};

class PatternGen : public Pattern {
public:
    PatternGen(Pattern *p, PatternGen *pg);
    virtual void printOn(std::ostream &out) const override;
    virtual SymbolEntry *sem_getExprObj() override;
    PatternGen *getNext();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;
    std::string getName();

private:
    Pattern *pattern;
    PatternGen *patternGen;
};

class PatternConstr : public Pattern {
public:
    PatternConstr(std::string id, PatternGen *pg);
    virtual void printOn(std::ostream &out) const override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

protected:
    std::string Id;
    PatternGen *patternGen;
};

class Clause : public Expr {
public:
    Clause(Pattern *p, Expr *e);
    virtual void printOn(std::ostream &out) const override;
    Pattern *getPattern();
    Expr *getExpr();
    void setType(CustomType *t);
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    llvm::Value* patternCompile();
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Pattern *pattern;
    Expr *expr;
};

class BarClauseGen : public Expr {
public:
    BarClauseGen(Clause *c, BarClauseGen *bcg);
    virtual void printOn(std::ostream &out) const override;
    Clause *getClause();
    BarClauseGen *getBarClauseGen();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Clause *clause;
    BarClauseGen *barClauseGen;
};

class Match : public Expr {
public:
    Match(Expr *e, Clause *c, BarClauseGen *bcg);
    virtual void printOn(std::ostream &out) const override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *expr;
    Clause *clause;
    BarClauseGen *barClauseGen;
};

class For : public Expr, public Block {
public:
    For(char *id, Expr *s, Expr *end, Expr *e, bool isAscending);
    void printOn(std::ostream &out) const;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    char *id;
    Expr *start;
    Expr *end;
    Expr *expr;
    bool ascending;
};

class While : public Expr, public Block {
public:
    While(Expr *lc, Expr *e);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *loopCondition;
    Expr *expr;
};

class If : public Expr, public Block {
public:
    If(Expr *c, Expr *e1, Expr *e2);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *condition;
    Expr *expr1;
    Expr *expr2;
};

class Begin : public Expr, public Block {
public:
    Begin(Expr *e);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *expr;
};

class CommaExprGen : public Expr {
public:
    CommaExprGen(Expr *e, CommaExprGen *ceg);
    virtual void printOn(std::ostream &out) const override;
    CommaExprGen *getNext();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class Par : public Expr {
public:
    Par(std::string id, CustomType* t);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;
    void setInfo(std::pair<SymbolEntry *, int> fi);
    std::pair<SymbolEntry *, int> getInfo() const;

private:
    std::string id;
    CustomType *type;
    std::pair<SymbolEntry *, int> funcInfo;
};

class ParGen : public Expr {
public:
    ParGen(Par *p, ParGen *pg);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    ParGen *getNext();
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;
    void setInfo(std::pair<SymbolEntry *, int> fi);
    std::pair<SymbolEntry *, int> getInfo() const;

private:
    Par *par;
    ParGen *parGen;
    std::pair<SymbolEntry *, int> funcInfo;
};

class Def : public AST {
public:
    Def(std::string id, ParGen *pg, Expr *e, CustomType *t, CommaExprGen *ceg, bool isMutable);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

    std::string id;
    ParGen *parGen;
    Expr *expr;
    CustomType *type;
    CommaExprGen *commaExprGen;
    bool mut;
};

class DefGen : public AST {
public:
    DefGen(Def *d, DefGen *dg);
    virtual void printOn(std::ostream &out) const override;
    Def *getDef();
    DefGen *getNext();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

    Def *def;
    DefGen *defGen;
};

class Let : public Block {
public:
    Let(Def *d, DefGen *dg, bool isRec);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    std::set<std::string> getFreeVars(std::set<std::string> freeVars, SymbolEntry *se, bool eraseParams = true);
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

    Def *def;
    DefGen *defGen;
    std::vector<Def *> defs;
    std::vector<SymbolEntry *> defsSE;
    std::set<std::string> freeVars = {};
    bool rec;
};

class LetIn : public Expr, public Block {
public:
    LetIn(Let* l, Expr *e);
    virtual void printOn(std::ostream &out) const override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Let *let;
    Expr *expr;
    SymbolEntry *LetInSE;
};

class Delete : public Expr {
public:
    Delete(Expr *e);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *expr;
};

class New : public Expr {
public:
    New(CustomType *t);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

};

class ArrayItem : public Expr {
public:
    ArrayItem(std::string id, Expr *e, CommaExprGen *ceg);
    virtual void printOn(std::ostream &out) const override;
    std::string getName() override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

protected:
    std::string id;
    Expr *expr;
    CommaExprGen *commaExprGen;
};

class Dim : public Expr {
public:
    Dim(std::string id);
    Dim(std::string id, int ic);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    std::string id;
    int intconst;
};

class BinOp : public Expr {
public:
    BinOp(Expr *e1, const char * op, Expr *e2);
    virtual void printOn(std::ostream &out) const override;
    virtual SymbolEntry *sem_getExprObj();
    virtual void sem() override;
    llvm::Value *generalTypeCheck(llvm::Value *val1, llvm::Value *val2, CustomType* ct);
    llvm::Function *constrsEqCheck(llvm::Value *constr1, llvm::Value *constr2, SymbolEntry *baseTypeSE);
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Expr *expr1;
    const char * op;
    Expr *expr2;
};

class UnOp : public Expr {
public:
    UnOp(const char * op, Expr *e);
    virtual void printOn(std::ostream &out) const override;
    std::string getName() override;
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    const char * op;
    Expr *expr;
};

class IntConst : public Constant, public Pattern {
public:
    IntConst(int ic, bool b = false);
    IntConst(int ic, char s, bool b = false);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    int intConst;
    bool isPattern;
};

class FloatConst : public Constant, public Pattern {
public:
    FloatConst(double fc, bool b = false);
    FloatConst(double fc, const char * s, bool b = false);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    double floatConst;
    bool isPattern;
};

class CharConst : public Constant, public Pattern {
public:
    CharConst(std::string cc, bool b = false);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    char charConst;
    bool isPattern;
};

class StringLiteral : public Constant, public Expr {
public:
    StringLiteral(std::string sl);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    std::string stringLiteral;
};

class BooleanConst : public Constant, public Pattern {
public:
    BooleanConst(bool b, bool bp = false);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    const bool boolean;
    bool isPattern;
};

class UnitConst : public Constant, public Expr {
public:
    UnitConst();
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

};

class TypeGen : public AST {
public:
    TypeGen(CustomType *t, TypeGen *tg);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

    CustomType *type;
    TypeGen *typeGen;
};

class Constr : public Expr {
public:
    Constr(std::string id, TypeGen *tg);
    Constr(std::string id, Expr *e, ExprGen *eg);
    virtual void printOn(std::ostream &out) const override;
    std::string getName();
    virtual SymbolEntry *sem_getExprObj() override;
    virtual void sem() override;
    void defineConstr(SymbolEntry *se);
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    std::string Id;
    TypeGen *typeGen = nullptr;
    Expr *expr = nullptr;
    ExprGen *exprGen = nullptr;
    bool call;
};

class BarConstrGen : public AST {
public:
    BarConstrGen(Constr *c, BarConstrGen *bcg);
    virtual void printOn(std::ostream &out) const override;
    Constr *getConstr();
    BarConstrGen *getNext();
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Constr *constr;
    BarConstrGen *barConstrGen;
};

class Tdef : public AST {
public:
    Tdef(std::string id, Constr *c, BarConstrGen *bcg);
    virtual void printOn(std::ostream &out) const override;
    std::string getName();
    BarConstrGen *getBarConstrGen();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    std::string id;
    Constr *constr;
    BarConstrGen *barConstrGen;
};

class TdefGen : public AST {
public:
    TdefGen(Tdef *td, TdefGen *tdg);
    virtual void printOn(std::ostream &out) const override;
    TdefGen *getNext();
    Tdef *getTdef();
    virtual void sem() override;
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Tdef *tDef;
    TdefGen *tDefGen;
};

class TypeDef : public Block {
public:
    TypeDef(Tdef *td, TdefGen *tdg);
    virtual void printOn(std::ostream &out) const override;
    virtual void sem() override;
    void defineUDT(Tdef *td);
    virtual std::set<std::string> preCompile();
    virtual llvm::Value* compile() override;

private:
    Tdef *tDef;
    TdefGen *tDefGen;
};

#endif