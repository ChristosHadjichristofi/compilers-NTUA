#include "ast.hpp"

/************************************/
/*               BLOCK              */
/************************************/

void Block::printOn(std::ostream &out) const {

    out << "Block(";
    bool first = true;
    for (auto i = block.rbegin(); i != block.rend(); ++i) {
        if (!first) out << ", ";
        first = false;
        (*i)->printOn(out);
    }
    out<< ")";
}

/************************************/
/*              EXRP GEN            */
/************************************/

void ExprGen::printOn(std::ostream &out) const {

    if (exprGen == nullptr) {
        out << "ExprGen("; expr->printOn(out); out <<")";
    }
    else {
        out << "ExprGen("; expr->printOn(out); out << ", "; exprGen->printOn(out); out << ")";
    }

}

/************************************/
/*               ID                 */
/************************************/

void Id::printOn(std::ostream &out) const {
    if (exprGen == nullptr) {
        out << "Id(" << name;
        if (expr != nullptr) { out << ", "; expr->printOn(out); }
        out << ")";
    }
    else {
        out << "Id(" << name <<", "; expr->printOn(out); out <<", "; exprGen->printOn(out); out <<")";
    }
}

/************************************/
/*            PATTERN ID            */
/************************************/

void PatternId::printOn(std::ostream &out) const {
    out << "PatternId(" << name <<")";
}

/************************************/
/*            PATTERN GEN           */
/************************************/

void PatternGen::printOn(std::ostream &out) const {
    if (patternGen == nullptr) {
        out << "PatternGen("; pattern->printOn(out); out << ")" ;
    }
    else {
        out << "PatternGen("; pattern->printOn(out); out << ", "; patternGen->printOn(out); out << ")";
    }
}

/************************************/
/*          PATTERN CONSTR          */
/************************************/

void PatternConstr::printOn(std::ostream &out) const {
    if (patternGen == nullptr) {
        out << "PatternConstrId(" << Id << ")";
    }
    else {
        out << "PatternConstrId(" << Id << ", "; patternGen->printOn(out); out << ")";
    }
}

/************************************/
/*              CLAUSE              */
/************************************/

void Clause::printOn(std::ostream &out) const {

    out << "Clause("; pattern->printOn(out); out << ", "; expr->printOn(out); out <<")";

}

/************************************/
/*           BAR CLAUSE GEN         */
/************************************/

void BarClauseGen::printOn(std::ostream &out) const {

    if (barClauseGen == nullptr) {
        out << "BarClauseGen("; clause->printOn(out); out <<")";
    }
    else {
        out << "BarClauseGen("; clause->printOn(out); out << ", "; barClauseGen->printOn(out); out <<")";
    }

}

/************************************/
/*               MATCH              */
/************************************/

void Match::printOn(std::ostream &out) const {

    out << "Match("; expr->printOn(out); out << ", "; clause->printOn(out);
    if (barClauseGen != nullptr) { out << ", "; barClauseGen->printOn(out); }
    out << ")";

}

/************************************/
/*                FOR               */
/************************************/

void For::printOn(std::ostream &out) const {

    out << "For("<< id <<", "; start->printOn(out); out <<", "; end->printOn(out); out <<", "; expr->printOn(out); out <<", " << ascending <<")";

}

/************************************/
/*              WHILE               */
/************************************/

void While::printOn(std::ostream &out) const {

    out << "While("; loopCondition->printOn(out); out <<", "; expr->printOn(out); out <<")";
}

/************************************/
/*                IF                */
/************************************/

void If::printOn(std::ostream &out) const {

    if (expr2 == nullptr) {
        out << "If("; condition->printOn(out); out << ", "; expr1->printOn(out); out << ")";
    }
    else {
        out << "If("; condition->printOn(out); out << ", "; expr1->printOn(out); out << ", "; expr2->printOn(out); out << ")";
    }
}

/************************************/
/*               BEGIN              */
/************************************/

void Begin::printOn(std::ostream &out) const {

    out << "Begin("; expr->printOn(out); out <<")";

}

/************************************/
/*          COMMA EXPR GEN          */
/************************************/

void CommaExprGen::printOn(std::ostream &out) const {

    if (commaExprGen == nullptr) {
        out << "CommaExprGen("; expr->printOn(out); out << ")";
    }
    else {
        out << "CommaExprGen("; expr->printOn(out); out << ", "; commaExprGen->printOn(out); out << ")";
    }

}

/************************************/
/*               PAR                */
/************************************/

void Par::printOn(std::ostream &out) const {

    if (type == nullptr) {
        out << "Par(" << id << ")";
    }
    else {
        out << "Par(" << id << ", "; type->printOn(out); out << ")" ;
    }

}

/************************************/
/*             PAR GEN              */
/************************************/

void ParGen::printOn(std::ostream &out) const {

    if (parGen == nullptr) {
        out << "ParGen("; par->printOn(out); out << ")";
    }
    else {
        out << "ParGen("; par->printOn(out); out << ", "; parGen->printOn(out); out << ")";
    }

}

/************************************/
/*                DEF               */
/************************************/

void Def::printOn(std::ostream &out) const {

    if (mut) { out << "MutableDef(" << id; }
    else { out << "Def(" << id; }
    if (parGen != nullptr) { out << ", "; parGen->printOn(out); }
    if (expr != nullptr) { out << ", "; expr->printOn(out); }
    if (type != nullptr) { out << ", "; type->printOn(out); }
    if (commaExprGen != nullptr) { out <<", "; commaExprGen->printOn(out); }
    out << ")";
}

/************************************/
/*             DEF GEN              */
/************************************/

void DefGen::printOn(std::ostream &out) const {

    if (defGen == nullptr) {
        out << "DefGen("; def->printOn(out); out << ")" ;
    }
    else {
        out << "DefGen("; def->printOn(out); out << ", "; defGen->printOn(out); out << ")";
    }
}

/************************************/
/*                LET               */
/************************************/

void Let::printOn(std::ostream &out) const {

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

/************************************/
/*              LETIN               */
/************************************/

void LetIn::printOn(std::ostream &out) const {

    out << "LetIn("; let->printOn(out); out <<", "; expr->printOn(out); out << ")";

}

/************************************/
/*              DELETE              */
/************************************/

void Delete::printOn(std::ostream &out) const {

    out << "Delete("; expr->printOn(out); out << ")";

}

/************************************/
/*                NEW               */
/************************************/

void New::printOn(std::ostream &out) const {

    out << "New("; type->printOn(out); out << ")";

}

/************************************/
/*             ARRAYITEM            */
/************************************/

void ArrayItem::printOn(std::ostream &out) const {

    if (commaExprGen == nullptr) {
        out << "ArrayItem("<< id << "["; expr->printOn(out); out << "])";
    }
    else {
        out <<"ArrayItem("<< id << "["; expr->printOn(out); out <<", "; commaExprGen->printOn(out); out << "])";
    }

}

/************************************/
/*                DIM               */
/************************************/

void Dim::printOn(std::ostream &out) const { out << "Dim(" << id << ", " << intconst << ")"; }

/************************************/
/*               BINOP              */
/************************************/

void BinOp::printOn(std::ostream &out) const {

    out << "BinOp("; expr1->printOn(out); out << ", " << op << ", "; expr2->printOn(out); out << ")";

}

/************************************/
/*                UNOP              */
/************************************/

void UnOp::printOn(std::ostream &out) const {

    out << "UnOp(" << op <<", "; expr->printOn(out); out <<")";

}

/************************************/
/*             INTCONST             */
/************************************/

void IntConst::printOn(std::ostream &out) const { out << intConst; }

/************************************/
/*            FLOATCONST            */
/************************************/

void FloatConst::printOn(std::ostream &out) const { out << floatConst; }

/************************************/
/*             CHARCONST            */
/************************************/

void CharConst::printOn(std::ostream &out) const { out << charConst; }

/************************************/
/*          STRINGLITERAL           */
/************************************/

void StringLiteral::printOn(std::ostream &out) const { out << stringLiteral; }

/************************************/
/*           BOOLEANCONST           */
/************************************/

void BooleanConst::printOn(std::ostream &out) const { (boolean) ? out << "true" : out << "false"; }

/************************************/
/*            UNITCONST             */
/************************************/

void UnitConst::printOn(std::ostream &out) const { out << "unit"; }

/************************************/
/*             TYPEGEN              */
/************************************/

void TypeGen::printOn(std::ostream &out) const {

    if (typeGen == nullptr) {
        out << "TypeGen("; type->printOn(out); out << ")";
    }
    else {
        out << "TypeGen("; typeGen->printOn(out); out << ", "; type->printOn(out); out << ")";
    }

}

/************************************/
/*              CONSTR              */
/************************************/

void Constr::printOn(std::ostream &out) const {
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

/************************************/
/*            BARCONSTRGEN          */
/************************************/

void BarConstrGen::printOn(std::ostream &out) const {

    if (barConstrGen == nullptr) {
        out << "BarConstrGen("; constr->printOn(out); out << ")";
    }
    else {
        out << "BarConstrGen("; constr->printOn(out); out << ", "; barConstrGen->printOn(out); out << ")";
    }

}

/************************************/
/*                TDEF              */
/************************************/

void Tdef::printOn(std::ostream &out) const {

    if (barConstrGen == nullptr) {
        out << "Tdef("<< id <<", "; constr->printOn(out); out << ")";
    }
    else {
        out << "Tdef("<< id <<", "; constr->printOn(out); out << ", "; barConstrGen->printOn(out); out << ")";
    }

}

/************************************/
/*              TDEFGEN             */
/************************************/

void TdefGen::printOn(std::ostream &out) const {

    if (tDefGen == nullptr) {
        out << "TdefGen("; tDef->printOn(out); out << ")";
    }
    else {
        out << "TdefGen("; tDef->printOn(out); out << ", "; tDefGen->printOn(out); out << ")";
    }

}

/************************************/
/*              TYPEDEF             */
/************************************/

void TypeDef::printOn(std::ostream &out) const {

    if (tDefGen == nullptr) {
        out << "TypeDef("; tDef->printOn(out); out << ")";
    }
    else {
        out << "TypeDef("; tDef->printOn(out); out << ", "; tDefGen->printOn(out); out << ")";
    }

}

/************************************/
/*       TYPES.HPP - CUSTOMTYPE     */
/************************************/

void CustomType::printOn(std::ostream &out) const {
    if (!name.empty()) out << "CustomType(" << name << ")";
    else out << "CustomType()";
}

/************************************/
/*         TYPES.HPP - UNIT         */
/************************************/
  
void Unit::printOn(std::ostream &out) const {
    out << "Unit"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

/************************************/
/*        TYPES.HPP - INTEGER       */
/************************************/

void Integer::printOn(std::ostream &out) const {
    out << "Integer"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

/************************************/
/*        TYPES.HPP - CHARACTER     */
/************************************/

void Character::printOn(std::ostream &out) const {
    out << "Character"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

/************************************/
/*        TYPES.HPP - BOOLEAN       */
/************************************/

void Boolean::printOn(std::ostream &out) const {
    out << "Boolean"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

/************************************/
/*          TYPES.HPP - FLOAT       */
/************************************/

void Float::printOn(std::ostream &out) const {
    out << "Float"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

/************************************/
/*        TYPES.HPP - FUNCTION      */
/************************************/

void Function::printOn(std::ostream &out) const {
    out << "fn: ";
    if (!params.empty()) 
        for (auto i : params) { i->printOn(out); out << " -> "; } 
    outputType->printOn(out);
}

/************************************/
/*        TYPES.HPP - REFERENCE     */
/************************************/

void Reference::printOn(std::ostream &out) const {
    out << "Reference"; 
    if(ofType != nullptr) {
        out <<" of type "; ofType->printOn(out); if (SHOW_MEM) out << "  MEM:  " << ofType;
    }
}

/************************************/
/*         TYPES.HPP - ARRAY        */
/************************************/

void Array::printOn(std::ostream &out) const {
    // out << "Array(ofType"; ofType->printOn(out); out <<", size:" << size <<")";
    out << "Array [";
    for (int i = 0; i < size - 1; i++) out << "*,";
    out << "*] of type "; 
    ofType->printOn(out);
}

/************************************/
/*         TYPES.HPP - CUSTOMID     */
/************************************/

void CustomId::printOn(std::ostream &out) const {
    out << name << "("; 
    if(!params.empty()) {
        bool first = true;
        for (auto p : params) { 
        if(first) first = false;
        else out << ", "; 
        p->printOn(out);
        }
    }
    out << ")";
}

/************************************/
/*        TYPES.HPP - UNKNOWN       */
/************************************/
   
void Unknown::printOn(std::ostream &out) const {
    if (size == -1) out << "Unknown"; else out << "None";
    if (SHOW_MEM) out << this;
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}