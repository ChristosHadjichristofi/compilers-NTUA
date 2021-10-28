#include "ast.hpp"
#include <set>
#include <algorithm>

/************************************/
/*              PATTERN             */
/************************************/

std::set<std::string> Pattern::preCompile() {
    return {};
}

/************************************/
/*               BLOCK              */
/************************************/

std::set<std::string> Block::preCompile() {
    for (auto i = block.rbegin(); i != block.rend(); ++i) (*i)->preCompile();
    return {};
}

/************************************/
/*              EXRP GEN            */
/************************************/

std::set<std::string> ExprGen::preCompile() {

    std::set<std::string> s1 = expr->preCompile();
    if (exprGen != nullptr) {
        std::set<std::string> s2 = exprGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    }
    return s1;

}

/************************************/
/*               ID                 */
/************************************/

std::set<std::string> Id::preCompile() {
    
    std::set<std::string> s1 = {name};
    if (expr != nullptr) {
        std::set<std::string> s2 = expr->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
        if (exprGen != nullptr) {
            std::set<std::string> s3 = exprGen->preCompile();
            std::set_union(s1.begin(), s1.end(), s3.begin(), s3.end(), std::inserter(s1, s1.begin()));    
        }
    }
    return s1;
}

/************************************/
/*            PATTERN ID            */
/************************************/

std::set<std::string> PatternId::preCompile() {
    return {name};
}

/************************************/
/*            PATTERN GEN           */
/************************************/

std::set<std::string> PatternGen::preCompile() {
    std::set<std::string> s1 = pattern->preCompile();
    if (patternGen != nullptr) {
        std::set<std::string> s2 = patternGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    }
    return s1;

}

/************************************/
/*          PATTERNCONSTR          */
/************************************/

std::set<std::string> PatternConstr::preCompile() {
    if (patternGen == nullptr) return {};
    return patternGen->preCompile();
}

/************************************/
/*              CLAUSE              */
/************************************/

std::set<std::string> Clause::preCompile() {
    return expr->preCompile();
}

/************************************/
/*           BAR CLAUSE GEN         */
/************************************/

std::set<std::string> BarClauseGen::preCompile() {

    std::set<std::string> s1 = clause->preCompile();
    if (barClauseGen != nullptr) {
        std::set<std::string> s2 = barClauseGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    }
    return s1;
}

/************************************/
/*               MATCH              */
/************************************/

std::set<std::string> Match::preCompile() {
    std::set<std::string> s1 = expr->preCompile();
    std::set<std::string> s2 = clause->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    if (barClauseGen != nullptr) {
        std::set<std::string> s2 = barClauseGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    }
    std::set<std::string> sp1 = clause->getPattern()->preCompile();
    BarClauseGen *tempBarClauseGen = barClauseGen;
    std::set<std::string> sp2;
    while (tempBarClauseGen != nullptr) {
        sp2 = tempBarClauseGen->getClause()->getPattern()->preCompile();
        std::set_union(sp1.begin(), sp1.end(), sp2.begin(), sp2.end(), std::inserter(sp1, sp1.begin()));
        tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
    }
    for (auto entry : sp1) {
        auto it = s1.find(entry);
        if (it != s1.end()) s1.erase(it);
    }
    return s1;
}

/************************************/
/*                FOR               */
/************************************/

std::set<std::string> For::preCompile() {

    std::set<std::string> s1 = start->preCompile();
    std::set<std::string> s2 = end->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    std::set<std::string> s3 = expr->preCompile();
    std::set_union(s1.begin(), s1.end(), s3.begin(), s3.end(), std::inserter(s1, s1.begin()));
    if (s1.find(id) != s1.end())
        s1.erase(s1.find(id));
    return s1;
}

/************************************/
/*              WHILE               */
/************************************/

std::set<std::string> While::preCompile() {
    
    std::set<std::string> s1 = loopCondition->preCompile();
    std::set<std::string> s2 = expr->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    return s1;
}

/************************************/
/*                IF                */
/************************************/

std::set<std::string> If::preCompile() {

    std::set<std::string> s1 = condition->preCompile();
    std::set<std::string> s2 = expr1->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    if (expr2 != nullptr) {
        std::set<std::string> s3 = expr2->preCompile();
        std::set_union(s1.begin(), s1.end(), s3.begin(), s3.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*               BEGIN              */
/************************************/

std::set<std::string> Begin::preCompile() {
    return expr->preCompile();
}

/************************************/
/*          COMMA EXPR GEN          */
/************************************/

std::set<std::string> CommaExprGen::preCompile() {

    std::set<std::string> s1 = expr->preCompile();
    if (commaExprGen != nullptr) {
        std::set<std::string> s2 = commaExprGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*               PAR                */
/************************************/

std::set<std::string> Par::preCompile() {
    return {};
}

/************************************/
/*             PAR GEN              */
/************************************/

std::set<std::string> ParGen::preCompile() {
    return {};
}

/************************************/
/*                DEF               */
/************************************/

std::set<std::string> Def::preCompile() {

    std::set<std::string> s1 = {};
    if (expr != nullptr) {
        std::set<std::string> s2 = expr->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    }
    if (commaExprGen != nullptr) {
        std::set<std::string> s3 = commaExprGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s3.begin(), s3.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*             DEF GEN              */
/************************************/

std::set<std::string> DefGen::preCompile() {

    std::set<std::string> s1 = def->preCompile();
    if (defGen != nullptr) {
        std::set<std::string> s2 = defGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*                LET               */
/************************************/

std::set<std::string> Let::preCompile() {

    std::set<std::string> s1 = def->preCompile();
    if (defGen != nullptr) {
        std::set<std::string> s2 = defGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    }
    freeVars = s1;
    return {};

}

/************************************/
/*              LETIN               */
/************************************/

std::set<std::string> LetIn::preCompile() {

    std::set<std::string> s1 = let->preCompile();
    std::set<std::string> s2 = expr->preCompile();
    // std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    return {};

}

/************************************/
/*              DELETE              */
/************************************/

std::set<std::string> Delete::preCompile() {
    return expr->preCompile();
}

/************************************/
/*                NEW               */
/************************************/

std::set<std::string> New::preCompile() {
    return {};
}

/************************************/
/*             ARRAYITEM            */
/************************************/

std::set<std::string> ArrayItem::preCompile() {

    std::set<std::string> s1 = {id};
    std::set<std::string> s2 = expr->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    if (commaExprGen != nullptr) {
        std::set<std::string> s3 = commaExprGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s3.begin(), s3.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*                DIM               */
/************************************/

std::set<std::string> Dim::preCompile() {

    std::set<std::string> s1 = {id};
    return s1;
}

/************************************/
/*               BINOP              */
/************************************/

std::set<std::string> BinOp::preCompile() {
    
    std::set<std::string> s1 = expr1->preCompile();
    std::set<std::string> s2 = expr2->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    return s1;
}

/************************************/
/*                UNOP              */
/************************************/

std::set<std::string> UnOp::preCompile() {
    return expr->preCompile();
}

/************************************/
/*             INTCONST             */
/************************************/

std::set<std::string> IntConst::preCompile() { return {}; }

/************************************/
/*            FLOATCONST            */
/************************************/

std::set<std::string> FloatConst::preCompile() { return {}; }

/************************************/
/*             CHARCONST            */
/************************************/

std::set<std::string> CharConst::preCompile() { return{}; }

/************************************/
/*          STRINGLITERAL           */
/************************************/

std::set<std::string> StringLiteral::preCompile() { return {}; }

/************************************/
/*           BOOLEANCONST           */
/************************************/

std::set<std::string> BooleanConst::preCompile() { return {}; }

/************************************/
/*            UNITCONST             */
/************************************/

std::set<std::string> UnitConst::preCompile() { return {}; }

/************************************/
/*             TYPEGEN              */
/************************************/

std::set<std::string> TypeGen::preCompile() { return {}; }

/************************************/
/*             CONSTR              */
/************************************/

std::set<std::string>Constr::preCompile() {

    std::set<std::string> s1 = {};
    if (expr != nullptr) {
        s1 = expr->preCompile();
        if (exprGen != nullptr) {
            std::set<std::string> s2 = exprGen->preCompile();
            std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
        }
    }
    return s1;
}

/************************************/
/*            BARCONSTRGEN          */
/************************************/

std::set<std::string> BarConstrGen::preCompile() {

    std::set<std::string> s1 = constr->preCompile();
    if (barConstrGen != nullptr) {
        std::set<std::string> s2 = barConstrGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*                TDEF              */
/************************************/

std::set<std::string> Tdef::preCompile() {

    std::set<std::string> s1 = constr->preCompile();
    if (barConstrGen != nullptr) {
        std::set<std::string> s2 = barConstrGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*              TDEFGEN             */
/************************************/

std::set<std::string> TdefGen::preCompile() {

    std::set<std::string> s1 = tDef->preCompile();
    if (tDefGen != nullptr) {
        std::set<std::string> s2 = tDefGen->preCompile();
        std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
    }
    return s1;
}

/************************************/
/*              TYPEDEF             */
/************************************/

std::set<std::string> TypeDef::preCompile() {

    return {};
}
