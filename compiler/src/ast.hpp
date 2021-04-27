#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>

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
    Def(std::string id, ParGen *pg, Expr *e, Type *t, CommaExprGen *ceg, bool isMutable): 
        id(id), parGen(pg), expr(e), type(t), commaExprGen(ceg), mut(isMutable) {}

    virtual void printOn(std::ostream &out) const override {

        if (mut) out << "MutableDef(" << id; else out << "Def(" << id;
        if (parGen != nullptr) out <<", " << *parGen;
        if (expr != nullptr) out <<", " << *expr;
        if (type != nullptr) out <<", " << *type;
        if (commaExprGen != nullptr) out <<", " << *commaExprGen;
        out << ")";
    }

private:
    std::string id;
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



#endif