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
    currPseudoScope = currPseudoScope->getNext();
    currPseudoScope = currPseudoScope->getNext();
    for (auto i = block.rbegin(); i != block.rend(); ++i) (*i)->preCompile();
    currPseudoScope->initCurrIndexes();
    currPseudoScope = currPseudoScope->getPrev();
    currPseudoScope->currIndex--;
    currPseudoScope = currPseudoScope->getPrev();
    currPseudoScope->currIndex--;
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
    PatternGen *tempPatternGen = patternGen;
    while (tempPatternGen != nullptr) {
        if (tempPatternGen->getName().compare("")) pseudoST.incrSize();
        tempPatternGen = tempPatternGen->getNext();
    }
    if (patternGen == nullptr) return {};
    return patternGen->preCompile();
}

/************************************/
/*              CLAUSE              */
/************************************/

std::set<std::string> Clause::preCompile() {
    currPseudoScope = currPseudoScope->getNext();
    auto s = expr->preCompile();
    currPseudoScope = currPseudoScope->getPrev();
    return s;
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
    currPseudoScope = currPseudoScope->getNext();
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
    currPseudoScope = currPseudoScope->getPrev();
    return s1;
}

/************************************/
/*                FOR               */
/************************************/

std::set<std::string> For::preCompile() {
    pseudoST.incrSize();
    currPseudoScope = currPseudoScope->getNext();
    std::set<std::string> s1 = start->preCompile();
    std::set<std::string> s2 = end->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    std::set<std::string> s3 = expr->preCompile();
    std::set_union(s1.begin(), s1.end(), s3.begin(), s3.end(), std::inserter(s1, s1.begin()));
    if (s1.find(id) != s1.end())
        s1.erase(s1.find(id));
    currPseudoScope = currPseudoScope->getPrev();
    return s1;
}

/************************************/
/*              WHILE               */
/************************************/

std::set<std::string> While::preCompile() {
    currPseudoScope = currPseudoScope->getNext();
    std::set<std::string> s1 = loopCondition->preCompile();
    std::set<std::string> s2 = expr->preCompile();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    currPseudoScope = currPseudoScope->getPrev();
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
    currPseudoScope = currPseudoScope->getNext();
    auto s = expr->preCompile();
    currPseudoScope = currPseudoScope->getPrev();
    return s;
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
    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize()+1);
    if (se != nullptr) pseudoST.incrSize(); // increase size only after veryfying it is in ST
    return {};
}

/************************************/
/*             PAR GEN              */
/************************************/

std::set<std::string> ParGen::preCompile() {
    par->preCompile();
    if (parGen != nullptr) parGen->preCompile();
    return {};
}

/************************************/
/*                DEF               */
/************************************/

std::set<std::string> Def::preCompile() {
    /* increase size of pseudoST for a new variable that was inserted */
    pseudoST.incrSize();
    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
    if(!mut) {
        /* if def is a function */
        if (parGen != nullptr) {
            int index = 0;
            ParGen *tempParGen = parGen;
            while (tempParGen != nullptr) {
                se->params.at(index++)->isVisible = true;
                /* increase size of pseudoST for a new function param that was inserted */
                pseudoST.incrSize();
                tempParGen = tempParGen->getNext();
            }
            currPseudoScope = currPseudoScope->getNext();
            currPseudoScope = currPseudoScope->getPrev();
        }
    }
    return {};
}

/************************************/
/*             DEF GEN              */
/************************************/

std::set<std::string> DefGen::preCompile() {

    def->preCompile();
    if (defGen != nullptr) defGen->preCompile();
    return {};
}

/************************************/
/*                LET               */
/************************************/

std::set<std::string> Let::getFreeVars(std::set<std::string> freeVars, SymbolEntry *se) {
    /* erase current function from freeVars (if it's rec) */
    auto it = freeVars.find(se->id);
    if (it != freeVars.end()) freeVars.erase(it);

    /* erase all library functions */
    for (auto entry : libraryVars) {
        auto it = freeVars.find(entry);
        if (it != freeVars.end()) freeVars.erase(it);
    }

    /* erase all params that are already given */
    for (auto p : se->params) {
        it = freeVars.find(p->id);
        if (it != freeVars.end()) freeVars.erase(it);
    }

    /* erase all functios that have been defined and exist in llvm global scope */
    std::set<std::string> funcVarsSet = {};
    for (auto entry : freeVars)
        if (currPseudoScope->lookup(entry, pseudoST.getSize())->entryType == ENTRY_FUNCTION) funcVarsSet.insert(entry);
        
    for (auto entry : funcVarsSet)
        freeVars.erase(freeVars.find(entry));

    return freeVars;
}

std::set<std::string> Let::preCompile() {
    
    def->preCompile();
    if (defGen != nullptr) defGen->preCompile();
    std::set<std::string> s1 = {};
    for (auto currDef : defs) {
        if (currDef->mut || (!currDef->mut && currDef->parGen == nullptr)) {
            if (currDef->expr != nullptr)
                currDef->expr->preCompile();
        }
        /* if def is a function */
        else {
            currPseudoScope = currPseudoScope->getNext();
            
            currDef->parGen->preCompile();

            if (currDef->expr != nullptr) {
                std::set<std::string> s2 = currDef->expr->preCompile();
                std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
            }
            if (currDef->commaExprGen != nullptr) {
                std::set<std::string> s2 = currDef->commaExprGen->preCompile();
                std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));    
            }

            SymbolEntry *se = currPseudoScope->lookup(currDef->id, pseudoST.getSize());
            freeVars = getFreeVars(s1, se);
            
            for (auto fv : freeVars) {
                auto tempSE = currPseudoScope->lookup(fv, pseudoST.getSize());
                tempSE->isFreeVar = true;
            }

            // /* printing for debugging purposes */
            // std::cout <<"In Let for " <<se->id <<" and size is " <<freeVars.size() <<std::endl;
            // for (auto strFV : freeVars) {
            //     std::cout <<strFV <<std::endl;
            // }

            se->isVisible = true;
            currPseudoScope = currPseudoScope->getPrev();
        }
    }
    return {};

}

/************************************/
/*              LETIN               */
/************************************/

std::set<std::string> LetIn::preCompile() {
    currPseudoScope = currPseudoScope->getNext();
    std::set<std::string> s1 = let->preCompile();
    std::set<std::string> s2 = expr->preCompile();
    // std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(s1, s1.begin()));
    currPseudoScope = currPseudoScope->getPrev();
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
    if (!call) pseudoST.incrSize();
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
    pseudoST.incrSize();
    tDef->preCompile();
    if (tDefGen != nullptr) tDefGen->preCompile();
    return {};
}
