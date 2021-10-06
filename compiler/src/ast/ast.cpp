#include "ast.hpp"

llvm::LLVMContext AST::TheContext;
llvm::IRBuilder<> AST::Builder(TheContext);
std::unique_ptr<llvm::Module> AST::TheModule;
std::unique_ptr<llvm::legacy::FunctionPassManager> AST::TheFPM;

llvm::GlobalVariable *AST::TheNL;
llvm::Function *AST::TheWriteInteger;
llvm::Function *AST::TheWriteBoolean;
llvm::Function *AST::TheWriteChar;
llvm::Function *AST::TheWriteReal;
llvm::Function *AST::TheWriteString;
llvm::Function *AST::TheReadInteger;
llvm::Function *AST::TheReadBoolean;
llvm::Function *AST::TheReadChar;
llvm::Function *AST::TheReadReal;
llvm::Function *AST::TheReadString;
llvm::Function *AST::TheAbs;
llvm::Function *AST::TheFabs;
llvm::Function *AST::TheSqrt;
llvm::Function *AST::TheSin;
llvm::Function *AST::TheCos;
llvm::Function *AST::TheTan;
llvm::Function *AST::TheAtan;
llvm::Function *AST::TheExp;
llvm::Function *AST::TheLn;
llvm::Function *AST::ThePi;
llvm::Function *AST::TheIncr;
llvm::Function *AST::TheDecr;
llvm::Function *AST::TheFloatOfInt;
llvm::Function *AST::TheIntOfFloat;
llvm::Function *AST::TheRound;
llvm::Function *AST::TheIntOfChar;
llvm::Function *AST::TheCharOfInt;
llvm::Function *AST::TheStringLength;
llvm::Function *AST::TheStringCompare;
llvm::Function *AST::TheStringCopy;
llvm::Function *AST::TheStringConcat;
llvm::Function *AST::TheExit;

llvm::Type *AST::i1;
llvm::Type *AST::i8;
llvm::Type *AST::i32;
llvm::Type *AST::i64;
llvm::Type *AST::DoubleTyID;

/************************************/
/*               EXPR               */
/************************************/

std::string Expr::getName() { return ""; }

CustomType *Expr::getType() { return type; }

void Expr::setType(CustomType *t) { this->type = t; }


/************************************/
/*              BLOCK               */
/************************************/

Block::Block(): block() {}

void Block::appendBlock(Block *b) {
    block.push_back(b);
}

/************************************/
/*             EXPR GEN             */
/************************************/

ExprGen::ExprGen(Expr *e, ExprGen *eg): expr(e), exprGen(eg) {}

Expr *ExprGen::getExpr() { return expr; }

ExprGen *ExprGen::getNext() { return exprGen; }

/************************************/
/*               ID                 */
/************************************/

Id::Id(std::string n): name(n) {}

Id::Id(std::string n, Expr *e, ExprGen *eg): name(n), expr(e), exprGen(eg) {}

std::string Id::getName() { return name; }

/************************************/
/*            PATTERN ID            */
/************************************/

PatternId::PatternId(std::string id): name(id) {}

std::string PatternId::getName() { return name; }

/************************************/
/*            PATTERN GEN           */
/************************************/

PatternGen::PatternGen(Pattern *p, PatternGen *pg ): pattern(p), patternGen(pg) {}

PatternGen *PatternGen::getNext() { return patternGen; }

/************************************/
/*          PATTERN CONSTR          */
/************************************/

PatternConstr::PatternConstr(std::string id, PatternGen *pg): Id(id), patternGen(pg) {}

/************************************/
/*              CLAUSE              */
/************************************/

Clause::Clause(Pattern *p, Expr *e): pattern(p), expr(e) {}

Pattern *Clause::getPattern() { return pattern; }

Expr *Clause::getExpr() { return expr; }

void Clause::setType(CustomType *t) { this->type = t; }

/************************************/
/*           BAR CLAUSE GEN         */
/************************************/

BarClauseGen::BarClauseGen(Clause *c, BarClauseGen *bcg): clause(c), barClauseGen(bcg) {}

Clause *BarClauseGen::getClause() { return clause; }

BarClauseGen *BarClauseGen::getBarClauseGen() { return barClauseGen; }

/************************************/
/*               MATCH              */
/************************************/

Match::Match(Expr *e, Clause *c, BarClauseGen *bcg): expr(e), clause(c), barClauseGen(bcg) {}

/************************************/
/*                FOR               */
/************************************/

For::For(char *id, Expr *s, Expr *end, Expr *e, bool isAscending):
    id(id), start(s), end(end), expr(e), ascending(isAscending) {}

/************************************/
/*              WHILE               */
/************************************/

While::While(Expr *lc, Expr *e): loopCondition(lc), expr(e) {}

/************************************/
/*                IF                */
/************************************/

If::If(Expr *c, Expr *e1, Expr *e2): condition(c), expr1(e1), expr2(e2) {}

/************************************/
/*               BEGIN              */
/************************************/

Begin::Begin(Expr *e): expr(e) {}

/************************************/
/*          COMMA EXPR GEN          */
/************************************/

CommaExprGen::CommaExprGen(Expr *e, CommaExprGen *ceg): expr(e), commaExprGen(ceg) {}

CommaExprGen *CommaExprGen::getNext() { return commaExprGen; }

/************************************/
/*               PAR                */
/************************************/

Par::Par(std::string id, CustomType* t): id(id), type(t) {}

/************************************/
/*             PAR GEN              */
/************************************/

ParGen::ParGen(Par *p, ParGen *pg): par(p), parGen(pg) {}

ParGen *ParGen::getNext() { return parGen; }

/************************************/
/*                DEF               */
/************************************/

Def::Def(std::string id, ParGen *pg, Expr *e, CustomType *t, CommaExprGen *ceg, bool isMutable):
        id(id), parGen(pg), expr(e), type(t), commaExprGen(ceg), mut(isMutable) {}

/************************************/
/*             DEF GEN              */
/************************************/

DefGen::DefGen(Def *d, DefGen *dg): def(d), defGen(dg) {}

Def *DefGen::getDef() { return def; }

DefGen *DefGen::getNext() { return defGen; }

/************************************/
/*                LET               */
/************************************/

Let::Let(Def *d, DefGen *dg, bool isRec): def(d), defGen(dg), rec(isRec) {}

/************************************/
/*              LETIN               */
/************************************/

LetIn::LetIn(Let* l, Expr *e): let(l), expr(e) {}

/************************************/
/*              DELETE              */
/************************************/

Delete::Delete(Expr *e): expr(e) {}

/************************************/
/*                NEW               */
/************************************/

New::New(CustomType *t) { this->type = t; }

/************************************/
/*             ARRAYITEM            */
/************************************/

ArrayItem::ArrayItem(std::string id, Expr *e, CommaExprGen *ceg): id(id), expr(e), commaExprGen(ceg) {}

std::string ArrayItem::getName() { return id; }

/************************************/
/*                DIM               */
/************************************/

Dim::Dim(std::string id): id(id) { intconst = 1; }

Dim::Dim(std::string id, int ic): id(id), intconst(ic) {}

/************************************/
/*               BINOP              */
/************************************/

BinOp::BinOp(Expr *e1, const char * op, Expr *e2): expr1(e1), op(op), expr2(e2) {}

/************************************/
/*                UNOP              */
/************************************/

UnOp::UnOp(const char * op, Expr *e): op(op), expr(e)  {}

std::string UnOp::getName() { return expr->getName(); }

/************************************/
/*             INTCONST             */
/************************************/

IntConst::IntConst(int ic, bool b): isPattern(b) { intConst = ic; type = new Integer(); }

IntConst::IntConst(int ic, char s, bool b): isPattern(b) {
    intConst = (s == '+') ? ic : -ic;
    type = new Integer();
}

/************************************/
/*            FLOATCONST            */
/************************************/

FloatConst::FloatConst(double fc, bool b): isPattern(b) { floatConst = fc; type = new Float(); }

FloatConst::FloatConst(double fc, const char * s, bool b): isPattern(b) {
    floatConst = ( strcmp(s, "+.") == 0 ) ? fc : -fc;
    type = new Float();
}

/************************************/
/*             CHARCONST            */
/************************************/

CharConst::CharConst(std::string cc, bool b): isPattern(b) { 
    
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

/************************************/
/*          STRINGLITERAL           */
/************************************/

StringLiteral::StringLiteral(std::string sl): stringLiteral(sl) {
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

/************************************/
/*           BOOLEANCONST           */
/************************************/

BooleanConst::BooleanConst(bool b, bool bp): boolean(b), isPattern(bp) { type = new Boolean(); }

/************************************/
/*            UNITCONST             */
/************************************/

UnitConst::UnitConst() { type = new Unit(); }

/************************************/
/*             TYPEGEN              */
/************************************/

TypeGen::TypeGen(CustomType *t, TypeGen *tg): type(t), typeGen(tg) {}

/************************************/
/*              CONSTR              */
/************************************/

Constr::Constr(std::string id, TypeGen *tg): Id(id), typeGen(tg) { call = false; }

Constr::Constr(std::string id, Expr *e, ExprGen *eg): Id(id), expr(e), exprGen(eg) { call = true; }

std::string Constr::getName() { return Id; }

/************************************/
/*            BARCONSTRGEN          */
/************************************/

BarConstrGen::BarConstrGen(Constr *c, BarConstrGen *bcg): constr(c), barConstrGen(bcg) {}

Constr *BarConstrGen::getConstr() { return constr; }

BarConstrGen *BarConstrGen::getNext() { return barConstrGen; }

/************************************/
/*                TDEF              */
/************************************/

Tdef::Tdef(std::string id, Constr *c, BarConstrGen *bcg): id(id), constr(c), barConstrGen(bcg) {}

std::string Tdef::getName() { return id; }

BarConstrGen *Tdef::getBarConstrGen() { return barConstrGen; }

/************************************/
/*              TDEFGEN             */
/************************************/

TdefGen::TdefGen(Tdef *td, TdefGen *tdg): tDef(td), tDefGen(tdg) {}

TdefGen *TdefGen::getNext() { return tDefGen; }

Tdef *TdefGen::getTdef() { return tDef; }

/************************************/
/*              TYPEDEF             */
/************************************/

TypeDef::TypeDef(Tdef *td, TdefGen *tdg): tDef(td), tDefGen(tdg) {}