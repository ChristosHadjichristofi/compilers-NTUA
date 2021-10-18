#include "ast.hpp"
#include "../library/library.hpp"

bool semError = false;
std::vector<SymbolEntry *> recFunctions = {};

/************************************/
/*               EXPR               */
/************************************/
SymbolEntry *Expr::sem_getExprObj() { return nullptr; }

std::pair<CustomType *, int> Expr::getRefFinalType(CustomType *ct) const {

    int levels = 1;
    CustomType *obj = ct;

    while (obj->ofType != nullptr && obj->typeValue == TYPE_REF) {
        levels++;
        obj = obj->ofType;
    }
    if (levels == 1) return std::make_pair(ct, levels);
    return std::make_pair(obj, levels);

}

std::pair<CustomType *, int> Expr::getFnFinalType(CustomType *ct) const {

    int levels = 1;
    CustomType *obj = ct;

    while (obj->outputType != nullptr && obj->typeValue == TYPE_FUNC) {
        levels++;
        obj = obj->outputType;
    }
    if (levels == 1) return std::make_pair(ct, levels);
    return std::make_pair(obj, levels);

}

/************************************/
/*             EXRP GEN             */
/************************************/

void ExprGen::sem() {
    expr->sem();
    this->type = expr->getType();
    if (exprGen != nullptr) exprGen->sem();
}

/************************************/
/*               BLOCK              */
/************************************/

void Block::sem() {
    st.openScope();
    Library *l = new Library();
    l->init();
    st.openScope();
    for (auto i = block.rbegin(); i != block.rend(); ++i) (*i)->sem();
    st.closeScope();
    st.closeScope();
}

/************************************/
/*               ID                 */
/************************************/

SymbolEntry *Id::sem_getExprObj() { return st.lookup(name); }

void Id::sem() {

    /* lookup for variable, if exists */
    if (expr == nullptr && exprGen == nullptr) {
        SymbolEntry *tempEntry = st.lookup(name);
        if (tempEntry != nullptr) this->type = tempEntry->type;
        else {
            /* Print Error First Occurence */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;

            Error *err = new FirstOccurence(name);
            err->printError();

            tempEntry = new SymbolEntry(name, new Unknown());
            /* Now Unknown is recognized as None */
            tempEntry->type->size = 0;
            this->type = tempEntry->type;
            tempEntry->entryType = ENTRY_TEMP;
            st.insert(name, tempEntry);
        }
        if (isRec())
            for (int index = recFunctions.size()-1; index >= 0; index++)
                if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                    recFunctions.at(index)->type->outputType = this->type;
                    recFunctions.erase(recFunctions.begin() + index);
                    break;
                }
    }
    /* lookup for function */
    else {
        SymbolEntry *tempEntry = st.lookup(name);

        if (tempEntry != nullptr) {

            /* check if calling a non function */
            if (expr != nullptr && tempEntry->params.empty() && tempEntry->type->typeValue != TYPE_UNKNOWN) {
                CustomType *newFunc = new Function(new Unknown());
                ExprGen *tempExprGen = exprGen;
                expr->sem();
                newFunc->params.push_back(expr->getType());

                if (exprGen != nullptr) exprGen->sem();

                while (tempExprGen != nullptr) {
                    if (tempExprGen->getNext() == nullptr) newFunc->outputType = tempExprGen->getType();
                    else newFunc->params.push_back(tempExprGen->getType());
                    tempExprGen = tempExprGen->getNext();
                }

                /* Print Error */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(tempEntry->type, newFunc);
                err->printError();

                this->type = new Unknown();
                return;
            }

            if (tempEntry->type->typeValue == TYPE_UNKNOWN) {
                std::string tempName;
                int counter = 0;
                SymbolEntry *se;
                CustomType *ct = new Unknown();
                tempEntry->type->~CustomType();
                tempEntry->type = new (tempEntry->type) Function(new Unknown());

                if (expr != nullptr) {
                    tempName = tempEntry->id + "_param_" + std::to_string(counter++);
                    dynamic_cast<Function *>(tempEntry->type)->params.push_back(ct);
                    se = new SymbolEntry(tempName, ct);
                    se->entryType = ENTRY_TEMP;
                    tempEntry->params.push_back(se);
                }

                if (exprGen != nullptr) {
                    ExprGen *tempExprGen = exprGen;
                    while (tempExprGen != nullptr) {
                        ct = new Unknown();
                        tempName = tempEntry->id + "_param_" + std::to_string(counter++);
                        dynamic_cast<Function *>(tempEntry->type)->params.push_back(ct);
                        se = new SymbolEntry(tempName, ct);
                        se->entryType = ENTRY_TEMP;
                        tempEntry->params.push_back(se);
                        tempExprGen = tempExprGen->getNext();
                    }
                }
            }

            if (tempEntry->entryType == ENTRY_TEMP) {
                Error *err;
                CustomType *prevTempEntryType = new Unknown();
                prevTempEntryType->size = 0;
                this->type = prevTempEntryType;

                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                err = new TypeMismatch(prevTempEntryType, st.nonTempLookup(name)->type);
                err->printError();
                return;
            }

            this->type = dynamic_cast<Function*>(tempEntry->type)->outputType;
            /* lookup for first param of a function */
            if (expr != nullptr) expr->sem();

            /* type inference */
            if (tempEntry->params.front()->type->typeValue == TYPE_ARRAY
                && tempEntry->params.front()->type->ofType->typeValue == TYPE_UNKNOWN
                && expr->getType()->typeValue == TYPE_ARRAY
                && expr->getType()->ofType->typeValue != TYPE_UNKNOWN) {
                     // Destroy the object but leave the space allocated.
                CustomType *tempCT = tempEntry->params.front()->type->ofType;
                std::string tempName;
                if (expr->getType()->ofType->typeValue == TYPE_CUSTOM) {
                    SymbolEntry *exprObj = expr->sem_getExprObj();
                    tempName = exprObj->type->name;
                }
                tempCT->~CustomType();
                // Create a new object in the same space.
                if (expr->getType()->ofType->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                else if (expr->getType()->ofType->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                else if (expr->getType()->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                else if (expr->getType()->ofType->typeValue == TYPE_ARRAY && expr->getType()->ofType->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                else if (expr->getType()->ofType->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                else if (expr->getType()->ofType->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                else if (expr->getType()->ofType->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                else if (expr->getType()->ofType->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(expr->getType()->ofType->ofType);
                else if (expr->getType()->ofType->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = tempName; }
                    // tempEntry->params.front()->type->ofType = expr->getType()->ofType;
                }

            if (tempEntry->params.front()->type->typeValue == TYPE_ARRAY
                && tempEntry->params.front()->type->ofType->typeValue != TYPE_UNKNOWN
                && expr->getType()->typeValue == TYPE_ARRAY
                && expr->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                expr->setType(tempEntry->params.front()->type);
                SymbolEntry *se = expr->sem_getExprObj();
                se->type->ofType = expr->getType()->ofType;
            }
            if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->getType()->typeValue != TYPE_UNKNOWN) {
                // Destroy the object but leave the space allocated.
                CustomType *tempCT = tempEntry->params.front()->type;
                std::string tempName;
                if (expr->getType()->typeValue == TYPE_CUSTOM) {
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
                else if (expr->getType()->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                else if (expr->getType()->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(expr->getType()->ofType);
                else if (expr->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = tempName; }

                dynamic_cast<Function *>(tempEntry->type)->params.front() = tempEntry->params.front()->type;
            }

            /* edge case */
            if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->sem_getExprObj() != nullptr && expr->sem_getExprObj()->type->typeValue == TYPE_FUNC) {
                // tempEntry->params.front()->type = expr->sem_getExprObj()->type->outputType;
                expr->sem_getExprObj()->sameMemAsOutput = true;
                expr->sem_getExprObj()->type->outputType = tempEntry->params.front()->type;
                dynamic_cast<Function *>(tempEntry->type)->params.front() = tempEntry->params.front()->type;
            }

            if (tempEntry->params.front()->type->typeValue != TYPE_UNKNOWN
                && expr->getType()->typeValue == TYPE_UNKNOWN
                && expr->sem_getExprObj() != nullptr
                && (expr->sem_getExprObj()->type->typeValue == TYPE_UNKNOWN || (expr->sem_getExprObj()->type->typeValue == TYPE_ARRAY && expr->sem_getExprObj()->type->ofType->typeValue == TYPE_UNKNOWN) || getRefFinalType(expr->sem_getExprObj()->type).first->typeValue == TYPE_UNKNOWN)) {
                expr->setType(tempEntry->params.front()->type);

                SymbolEntry *se = expr->sem_getExprObj();

                if (se->type->typeValue == TYPE_ARRAY) se->type->ofType = expr->getType();
                else if (se->type->typeValue == TYPE_REF) {
                    CustomType * yt = se->type;
                    std::string tempName;

                    if (expr->getType()->typeValue == TYPE_CUSTOM) {
                        SymbolEntry *exprObj = expr->sem_getExprObj();
                        tempName = exprObj->type->name;
                    }

                    while (yt->typeValue == TYPE_REF) yt = yt->ofType;

                    yt->~CustomType();

                    if (expr->getType()->typeValue == TYPE_INT) yt = new (yt) Integer();
                    else if (expr->getType()->typeValue == TYPE_FLOAT) yt = new (yt) Float();
                    else if (expr->getType()->typeValue == TYPE_CHAR) yt = new (yt) Character();
                    else if (expr->getType()->typeValue == TYPE_ARRAY && expr->getType()->ofType->typeValue == TYPE_CHAR) yt = new (yt) Array(new Character(), 1);
                    else if (expr->getType()->typeValue == TYPE_BOOL) yt = new (yt) Boolean();
                    else if (expr->getType()->typeValue == TYPE_UNIT) yt = new (yt) Unit();
                    else if (expr->getType()->typeValue == TYPE_UNKNOWN) yt = new (yt) Unknown();
                    else if (expr->getType()->typeValue == TYPE_CUSTOM) { yt = new (yt) CustomType(); yt->name = tempName; }
                }
                else se->type = expr->getType();
            }

            if (tempEntry->params.front()->type->typeValue == TYPE_UNKNOWN && expr->getType()->typeValue == TYPE_UNKNOWN) {
                SymbolEntry *se = expr->sem_getExprObj();
                /* if expr is a function (as a param) and its return type unknown (common),
                    get expr->SymbolEntry and set the first param of tempEntry to its type */
                if (se->type->typeValue == TYPE_FUNC) {
                    /* edge case for same function being given as a param to itself
                        aka it is in the same memory */
                    if (!se->params.empty() && tempEntry->params.front() == se->params.front()) {}
                    else {
                        /* if another function call is given as a param must get its outputType */
                        tempEntry->params.front()->type = se->type->outputType;
                        tempEntry->type->params.front() = se->type->outputType;
                    }
                }
                else {
                    expr->setType(tempEntry->params.front()->type);
                    if (se->type->typeValue == TYPE_REF || se->type->typeValue == TYPE_ARRAY) se->type->ofType = expr->getType();
                    else se->type = expr->getType();
                }
            }

            /* Check first param of function with given param */
            if (expr->getType()->typeValue != tempEntry->params.front()->type->typeValue) {
                /* edge case for ref(unknown) to array(unknown) -> type correction */
                if (expr->getType()->typeValue == TYPE_ARRAY && expr->getType()->ofType->typeValue == TYPE_UNKNOWN
                && tempEntry->params.front()->type->typeValue == TYPE_REF && tempEntry->params.front()->type->ofType->typeValue == TYPE_UNKNOWN) {
                    tempEntry->params.front()->type = expr->getType();
                }
                // /* edge case for CustomId - CustomType type check */
                else if (expr->getType()->typeValue == TYPE_CUSTOM && tempEntry->params.front()->type->typeValue == TYPE_CUSTOM
                        && expr->sem_getExprObj()->params.front()->id == tempEntry->params.front()->type->name) {
                    /* all good */
                }
                /* edge case for function */
                else if (expr->getType()->typeValue == TYPE_FUNC || tempEntry->params.front()->type->typeValue == TYPE_FUNC) {
                    CustomType *tempFunc1 = expr->sem_getExprObj()->type;
                    CustomType *tempFunc2 = tempEntry->params.front()->type;
                    while (tempFunc1->typeValue == TYPE_FUNC) tempFunc1 = tempFunc1->outputType;
                    while (tempFunc2->typeValue == TYPE_FUNC) tempFunc2 = tempFunc2->outputType;
                    if (tempFunc1->typeValue != tempFunc2->typeValue) {
                        /* Print Error - type mismatch */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(tempFunc1, tempFunc2);
                        err->printError();
                    }
                }
                else{
                    /* Print Error - type mismatch */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(expr->getType(), tempEntry->params.front()->type);
                    err->printError();
                }
            }
            else {
                if (expr->getType()->typeValue == TYPE_ARRAY
                && tempEntry->params.front()->type->typeValue == TYPE_ARRAY
                && expr->getType()->size != tempEntry->params.front()->type->size) {
                    /* Print Error - type mismatch */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err = new ArrayDimensions(dynamic_cast<Array *>(expr->getType()), dynamic_cast<Array *>(tempEntry->params.front()->type));
                    err->printError();
                }
            }


            /* if a expr SE is a function, make sure all its params get type inference/check
                with the params of the function being called */
            if (expr->sem_getExprObj() != nullptr && expr->sem_getExprObj()->type->typeValue == TYPE_FUNC
                && tempEntry->params.front() != nullptr && !tempEntry->params.front()->id.empty()) {

                long unsigned int counter = 0;
                SymbolEntry *exprEntry = expr->sem_getExprObj();
                SymbolEntry *firstParamEntry = tempEntry->params.front();

                while (!firstParamEntry->params.empty() && counter < firstParamEntry->params.size()) {
                    /* type inference */
                    if ((firstParamEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN && dynamic_cast<Function *>(firstParamEntry->type)->params.at(counter)->typeValue == TYPE_UNKNOWN)
                    || firstParamEntry->params.at(counter)->sameMemAsOutput
                    ) {

                        CustomType *tempCT = firstParamEntry->params.at(counter)->type;
                        tempCT->~CustomType();

                        // Create a new object in the same space.
                        if (exprEntry->params.at(counter)->type->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_ARRAY && exprEntry->params.at(counter)->type->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(exprEntry->params.at(counter)->type->ofType);
                        else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FUNC) {
                            tempCT = new (tempCT) Function(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->outputType);

                            long unsigned int index = 0;

                            // iterating through dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params and
                            // adding them to type function params
                            while (index < dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.size())
                                dynamic_cast<Function *>(tempCT)->params.push_back(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.at(index++));

                        }
                    }
                    /* if param is a function it must change its (final) outputType */
                    if (firstParamEntry->params.at(counter)->type->typeValue == TYPE_FUNC) {
                        CustomType *tempFunc = firstParamEntry->params.at(counter)->type;
                        while (tempFunc->typeValue == TYPE_FUNC) tempFunc = tempFunc->outputType;
                        CustomType *exprCT = exprEntry->params.at(counter)->type;
                        tempFunc->~CustomType();

                        // Create a new object in the same space.
                        if (exprCT->typeValue == TYPE_INT) tempFunc = new (tempFunc) Integer();
                        else if (exprCT->typeValue == TYPE_FLOAT) tempFunc = new (tempFunc) Float();
                        else if (exprCT->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Character();
                        else if (exprCT->typeValue == TYPE_ARRAY && exprCT->ofType->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Array(new Character(), 1);
                        else if (exprCT->typeValue == TYPE_BOOL) tempFunc = new (tempFunc) Boolean();
                        else if (exprCT->typeValue == TYPE_UNIT) tempFunc = new (tempFunc) Unit();
                        else if (exprCT->typeValue == TYPE_UNKNOWN) tempFunc = new (tempFunc) Unknown();
                    }

                    /* type check */
                    CustomType *funcFinalType = firstParamEntry->params.at(counter)->type;

                    while (funcFinalType->typeValue == TYPE_FUNC) funcFinalType = funcFinalType->outputType;

                    if (funcFinalType->typeValue != exprEntry->params.at(counter)->type->typeValue) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(firstParamEntry->params.at(counter)->type, exprEntry->params.at(counter)->type);
                        err->printError();
                    }
                    counter++;
                }
                if (firstParamEntry->type->typeValue == TYPE_FUNC) {
                    // dynamic_cast<Function *>(firstParamEntry->type)->outputType = dynamic_cast<Function *>(exprEntry->type)->outputType;

                    CustomType *tempFunc = firstParamEntry->type->outputType;
                    CustomType *exprCT = exprEntry->type->outputType;

                    if (tempEntry->type->outputType->typeValue != TYPE_UNKNOWN) {
                        if (tempFunc->typeValue != exprCT->typeValue) {
                            /* Print Error - type mismatch */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(tempEntry->type->outputType, expr->sem_getExprObj()->type->outputType);
                        err->printError();
                        }
                    }
                    else {
                        tempFunc->~CustomType();

                        // Create a new object in the same space.
                        if (exprCT->typeValue == TYPE_INT) tempFunc = new (tempFunc) Integer();
                        else if (exprCT->typeValue == TYPE_FLOAT) tempFunc = new (tempFunc) Float();
                        else if (exprCT->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Character();
                        else if (exprCT->typeValue == TYPE_ARRAY && exprCT->ofType->typeValue == TYPE_CHAR) tempFunc = new (tempFunc) Array(new Character(), 1);
                        else if (exprCT->typeValue == TYPE_BOOL) tempFunc = new (tempFunc) Boolean();
                        else if (exprCT->typeValue == TYPE_UNIT) tempFunc = new (tempFunc) Unit();
                        else if (exprCT->typeValue == TYPE_UNKNOWN) tempFunc = new (tempFunc) Unknown();

                    }

                }
            }

            if (tempEntry->params.front()->type->typeValue == TYPE_REF || expr->getType()->typeValue == TYPE_REF) {
                std::pair <CustomType *, int> pairExpr1, pairExpr2;
                pairExpr1 = getRefFinalType(tempEntry->params.front()->type);
                pairExpr2 = getRefFinalType(expr->getType());
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
                        /* Print Error - type mismatch */

                        /* keep the depth of expr2 in order to know how much to traverse
                            into expr1 types and print the error correctly */
                        int pairExpr2Depth = pairExpr2.second;
                        CustomType *traverseType = tempEntry->params.front()->type;
                        while (pairExpr2Depth != 1) { traverseType = traverseType->ofType; pairExpr2Depth--; }

                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(traverseType, pairExpr2.first);
                        err->printError();
                    }
                    else tempEntry->params.front()->type = expr->getType();
                }
                else if (pairExpr2.first->typeValue == TYPE_UNKNOWN) {
                    if (pairExpr1.second < pairExpr2.second) {
                        /* Print Error - type mismatch */

                        /* keep the depth of expr1 in order to know how much to traverse
                            into expr2 types and print the error correctly */
                        int pairExpr1Depth = pairExpr1.second;
                        CustomType *traverseType = expr->getType();
                        while (pairExpr1Depth != 1) { traverseType = traverseType->ofType; pairExpr1Depth--; }

                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(traverseType, pairExpr1.first);
                        err->printError();
                    }
                    else {
                        expr->setType(tempEntry->params.front()->type);
                        SymbolEntry *se = dynamic_cast<Id*>(expr)->sem_getExprObj();
                        se->type = expr->getType();
                    }
                }
                /* Check ith param given that has the same type as the ith param of the function */
                else if (pairExpr1.first->typeValue != pairExpr2.first->typeValue || pairExpr1.second != pairExpr2.second ) {
                    /* Print Error - type mismatch */

                    int pairExprDepth;
                    CustomType *traverseType;

                    /* keep the depth of the expr that has the smallest depth
                        in order to know how much to traverse
                        into the other expr types and print the error correctly */
                    if (pairExpr1.second < pairExpr2.second) {
                        pairExprDepth = pairExpr1.second;
                        traverseType = expr->getType();
                    }
                    else {
                        pairExprDepth = pairExpr2.second;
                        traverseType = tempEntry->params.front()->type;
                    }

                    while (pairExprDepth != 1) { traverseType = traverseType->ofType; pairExprDepth--; }

                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;

                    Error *err;
                    if (pairExpr1.second < pairExpr2.second) err = new TypeMismatch(pairExpr1.first, traverseType);
                    else err = new TypeMismatch(traverseType, pairExpr2.first);
                    err->printError();
                }
            }

            /* if there is only expr (no other exprs) and it's a function,
                make this tempEntry's outputType equal to the outputType of expr */
            if (exprGen == nullptr && expr->sem_getExprObj() != nullptr && expr->sem_getExprObj()->type->typeValue == TYPE_FUNC && tempEntry->type->outputType->typeValue == TYPE_UNKNOWN) {
                tempEntry->type->outputType = expr->sem_getExprObj()->type->outputType;
            }


            /* lookup for the rest params of a function, if they exist */
            if (exprGen != nullptr) {
                exprGen->sem();
                /* Check for the rest params of function with the rest given params */
                ExprGen *tempExprGen = exprGen;
                long unsigned int i = 1;
                for (; (i < tempEntry->params.size() && tempExprGen != nullptr); i++, tempExprGen = tempExprGen->getNext()) {
                    
                    /* type inference for type array */
                    if (tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY
                    && tempEntry->params.at(i)->type->ofType->typeValue == TYPE_UNKNOWN
                    && tempExprGen->getExpr()->getType()->typeValue == TYPE_ARRAY
                    && tempExprGen->getExpr()->getType()->ofType->typeValue != TYPE_UNKNOWN) {
                        // Destroy the object but leave the space allocated.
                        CustomType *tempCT = tempEntry->params.at(i)->type->ofType;
                        std::string tempName;
                        if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_CUSTOM) {
                            SymbolEntry *exprObj = tempExprGen->getExpr()->sem_getExprObj();
                            tempName = exprObj->type->name;
                        }
                        tempCT->~CustomType();
                        // Create a new object in the same space.
                        if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_ARRAY && tempExprGen->getExpr()->getType()->ofType->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(tempExprGen->getExpr()->getType()->ofType->ofType);
                        else if (tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = tempName; }
                    }

                    if (tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY
                    && tempEntry->params.at(i)->type->ofType->typeValue != TYPE_UNKNOWN
                    && tempExprGen->getExpr()->getType()->typeValue == TYPE_ARRAY
                    && tempExprGen->getExpr()->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                        tempExprGen->getExpr()->setType(tempEntry->params.at(i)->type);
                        SymbolEntry *se = tempExprGen->getExpr()->sem_getExprObj();
                        se->type->ofType = tempExprGen->getExpr()->getType()->ofType;
                    }

                    /* Check if both function param and given param have unknown type */
                    if (tempEntry->params.at(i)->type->typeValue == TYPE_UNKNOWN && tempExprGen->getType()->typeValue == TYPE_UNKNOWN) {
                        tempExprGen->getExpr()->setType(tempEntry->params.at(i)->type);
                        SymbolEntry *exprGenEntry = tempExprGen->getExpr()->sem_getExprObj();
                        if (exprGenEntry->type->typeValue == TYPE_REF || exprGenEntry->type->typeValue == TYPE_ARRAY) exprGenEntry->type->ofType = tempExprGen->getExpr()->getType();
                        else exprGenEntry->type = tempExprGen->getExpr()->getType();
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
                        else if (tempExprGen->getType()->typeValue == TYPE_ARRAY) tempCT = new (tempCT) Array(tempExprGen->getType()->ofType, 1);
                        else if (tempExprGen->getType()->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                        else if (tempExprGen->getType()->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                        else if (tempExprGen->getType()->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                        else if (tempExprGen->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }

                        dynamic_cast<Function *>(tempEntry->type)->params.at(i) = tempEntry->params.at(i)->type;
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
                        else if (tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY) tempCT = new (tempCT) Array(tempEntry->params.at(i)->type->ofType, 1);
                        else if (tempEntry->params.at(i)->type->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                        else if (tempEntry->params.at(i)->type->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                        else if (tempEntry->params.at(i)->type->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                        else if (tempEntry->params.at(i)->type->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
                    }
                    /* Check ith param given that has the same type as the ith param of the function */
                    else {
                        
                    //     if (tempExprGen->getExpr()->sem_getExprObj() == nullptr) std::cout <<tempExprGen->getExpr()->getName() <<" is broken";
                    // tempExprGen->getExpr()->sem_getExprObj()->type->printOn(std::cout); std::cout.flush();
                        std::string nameOfType = "";
                        if (tempExprGen->getExpr()->sem_getExprObj() != nullptr) {
                            nameOfType = (tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_ID) ? tempExprGen->getExpr()->sem_getExprObj()->params.front()->type->name : tempExprGen->getExpr()->sem_getExprObj()->type->name;
                            if (tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_FUNC && tempExprGen->getExpr()->sem_getExprObj()->type->outputType->typeValue == TYPE_CUSTOM) nameOfType = tempExprGen->getExpr()->sem_getExprObj()->type->outputType->name;
                        }
                        
                        if (!nameOfType.compare("")) nameOfType = tempExprGen->getType()->name;
                        
                        if (tempExprGen->getType()->typeValue == TYPE_CUSTOM
                        && tempEntry->params.at(i)->type->typeValue == TYPE_CUSTOM
                        && tempEntry->params.at(i)->type->name != nameOfType) {
                            /* Print Error - type mismatch */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempEntry->params.at(i)->type, tempExprGen->getType());
                            err->printError();
                        }
                        else if (tempExprGen->getType()->typeValue != TYPE_CUSTOM && tempEntry->params.at(i)->type->typeValue != TYPE_CUSTOM) {
                            if (tempEntry->params.at(i)->type->typeValue != tempExprGen->getType()->typeValue) {
                                /* egde case for ref(unknown) to array(unknown) -> type correction */
                                if (tempExprGen->getType()->typeValue == TYPE_ARRAY && tempExprGen->getType()->ofType->typeValue == TYPE_UNKNOWN
                                && tempEntry->params.at(i)->type->typeValue == TYPE_REF && tempEntry->params.at(i)->type->ofType->typeValue == TYPE_UNKNOWN) {
                                    tempEntry->params.at(i)->type = expr->getType();
                                }
                                else{
                                    /* Print Error - type mismatch */
                                    semError = true;
                                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                    Error *err = new TypeMismatch(tempEntry->params.at(i)->type, tempExprGen->getType());
                                    err->printError();
                                }
                            }
                            else {
                                if (tempExprGen->getType()->typeValue == TYPE_ARRAY
                                && tempEntry->params.at(i)->type->typeValue == TYPE_ARRAY
                                && tempExprGen->getType()->size != tempEntry->params.at(i)->type->size) {
                                    /* Print Error - type mismatch */
                                    semError = true;
                                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                    Error *err = new ArrayDimensions(dynamic_cast<Array *>(tempExprGen->getType()), dynamic_cast<Array *>(tempEntry->params.at(i)->type));
                                    err->printError();
                                }
                            }
                        }
                    }

                    /* inference in params of function given as param */
                    if (tempExprGen->getExpr()->getType()->typeValue == TYPE_FUNC) {
                        long unsigned int counter = 0;
                        SymbolEntry *exprEntry = tempExprGen->getExpr()->sem_getExprObj();
                        SymbolEntry *paramEntry = tempEntry->params.at(i);
                        while (counter < paramEntry->params.size()) {
                            if (paramEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN
                            && dynamic_cast<Function *>(paramEntry->type)->params.at(counter)->typeValue == TYPE_UNKNOWN
                            && exprEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN
                            && dynamic_cast<Function *>(exprEntry->type)->params.at(counter)->typeValue == TYPE_UNKNOWN) {
                                exprEntry->params.at(counter)->type = paramEntry->params.at(counter)->type;
                                dynamic_cast<Function *>(exprEntry->type)->params.at(counter) = paramEntry->params.at(counter)->type;
                            }
                            counter++;
                        }
                    }

                    if (tempExprGen->getExpr()->getType()->typeValue == TYPE_FUNC) {

                        long unsigned int counter = 0;
                        SymbolEntry *exprEntry = tempExprGen->getExpr()->sem_getExprObj();
                        SymbolEntry *paramEntry = tempEntry->params.at(i);

                        while (counter < paramEntry->params.size()) {

                            /* type inference */
                            if (paramEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN
                                && dynamic_cast<Function *>(paramEntry->type)->params.at(counter)->typeValue == TYPE_UNKNOWN) {
                                CustomType *tempCT = paramEntry->params.at(counter)->type;
                                tempCT->~CustomType();

                                // Create a new object in the same space.
                                if (exprEntry->params.at(counter)->type->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_ARRAY && exprEntry->params.at(counter)->type->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                                else if (exprEntry->params.at(counter)->type->typeValue == TYPE_FUNC) {
                                    tempCT = new (tempCT) Function(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->outputType);

                                    long unsigned int index = 0;
                                    while (index < dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.size())
                                        dynamic_cast<Function *>(tempCT)->params.push_back(dynamic_cast<Function *>(exprEntry->params.at(counter)->type)->params.at(index++));
                                }
                                // dynamic_cast<Function *>(paramEntry->type)->params.at(counter) = paramEntry->params.at(counter)->type;
                            }

                            /* type check */
                            if (paramEntry->params.at(counter)->type->typeValue != exprEntry->params.at(counter)->type->typeValue) {
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(paramEntry->params.at(counter)->type, exprEntry->params.at(counter)->type);
                                err->printError();
                            }

                            counter++;
                        }

                        dynamic_cast<Function *>(paramEntry->type)->outputType = dynamic_cast<Function *>(exprEntry->type)->outputType;
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
                                /* Print Error - type mismatch */

                                /* keep the depth of expr2 in order to know how much to traverse
                                    into expr1 types and print the error correctly */
                                int pairExpr2Depth = pairExpr2.second;
                                CustomType *traverseType = tempEntry->params.at(i)->type;
                                while (pairExpr2Depth != 1) { traverseType = traverseType->ofType; pairExpr2Depth--; }

                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(traverseType, pairExpr2.first);
                                err->printError();
                            }
                            else tempEntry->params.at(i)->type = tempExprGen->getType();
                        }
                        else if (pairExpr2.first->typeValue == TYPE_UNKNOWN) {
                            if (pairExpr1.second < pairExpr2.second) {
                                /* Print Error - type mismatch */

                                /* keep the depth of expr1 in order to know how much to traverse
                                    into expr2 types and print the error correctly */
                                int pairExpr1Depth = pairExpr1.second;
                                CustomType *traverseType = tempExprGen->getType();
                                while (pairExpr1Depth != 1) { traverseType = traverseType->ofType; pairExpr1Depth--; }

                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(traverseType, pairExpr1.first);
                                err->printError();
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

                            int pairExprDepth;
                            CustomType *traverseType;

                            /* keep the depth of the expr that has the smallest depth
                                in order to know how much to traverse
                                into the other expr types and print the error correctly */
                            if (pairExpr1.second < pairExpr2.second) {
                                pairExprDepth = pairExpr1.second;
                                traverseType = tempExprGen->getType();
                            }
                            else {
                                pairExprDepth = pairExpr2.second;
                                traverseType = tempEntry->params.at(i)->type;
                            }

                            while (pairExprDepth != 1) { traverseType = traverseType->ofType; pairExprDepth--; }

                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;

                            Error *err;
                            if (pairExpr1.second < pairExpr2.second) err = new TypeMismatch(pairExpr1.first, traverseType);
                            else err = new TypeMismatch(traverseType, pairExpr2.first);
                            err->printError();
                        }
                    }
                }
                int extraParams = 0;
                CustomType *lastTypeMore;
                CustomType *lastTypeLess;
                std::vector<CustomType *> params;
                if (i < tempEntry->params.size()) {
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
                    extraParams--;
                    params.push_back(tempEntry->params.at(i)->type);
                    i++;
                }
                /* Params given are more than expected */
                if (extraParams > 0) {
                    CustomType *newFunc = new Function(new Unknown());
                    for (int j = 0; j < extraParams+1; j++) {
                        if (j == extraParams) dynamic_cast<Function *>(newFunc)->outputType = params.at(j);
                        else dynamic_cast<Function *>(newFunc)->params.push_back(params.at(j));
                    }
                    /* Print Error */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(tempEntry->params.back()->type, newFunc);
                    err->printError();
                }
                /* Params given are less than expected */
                if (extraParams < 0) {
                    CustomType *newFunc = new Function(new Unknown());
                    for (int j = 0; j < -(extraParams-1); j++) {
                        if (j == -extraParams) dynamic_cast<Function *>(newFunc)->outputType = params.at(j);
                        else dynamic_cast<Function *>(newFunc)->params.push_back(params.at(j));
                    }
                    /* Print Error */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err;
                    if (tempExprGen != nullptr) err = new TypeMismatch(newFunc, tempExprGen->getType());
                    else err = new TypeMismatch(newFunc, expr->getType());
                    err->printError();
                }
            }
            /* edge case - when calling a function with exactly 1 param but more are required */
            else {
                if (tempEntry->params.size() > 1) {
                    std::vector<CustomType *> params;
                    CustomType *newFunc = new Function(new Unknown());
                    for (long unsigned int j = 0; j < tempEntry->params.size(); j++) {
                        if (j == tempEntry->params.size()-1) dynamic_cast<Function *>(newFunc)->outputType = tempEntry->params.at(j)->type;
                        else dynamic_cast<Function *>(newFunc)->params.push_back(tempEntry->params.at(j)->type);
                    }
                    /* Print Error */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err;
                    err = new TypeMismatch(newFunc, expr->getType());
                    err->printError();
                }
            }
        }
        else {
            /* type should be set to unknown because when returned to the class
                that triggered this sem this->type will be null */
            /* Print Error First Occurence */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new FirstOccurence(name);
            err->printError();

            tempEntry = new SymbolEntry(name, new Unknown());
            /* Now Unknown is recognized as None */
            tempEntry->type->size = 0;
            this->type = tempEntry->type;
            tempEntry->entryType = ENTRY_TEMP;
            st.insert(name, tempEntry);
            this->sem();
        }
    }
}

/************************************/
/*            PATTERN ID            */
/************************************/

SymbolEntry *PatternId::sem_getExprObj() { return st.lookup(name); }

void PatternId::sem() {
    // /* lookup for variable if exists, if yes it's a duplicate error otherwise create  */
    SymbolEntry *tempEntry = st.lookup(name, true);
    if (tempEntry != nullptr) {
        semError = true;
        this->type = new Unknown();
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
        Error *err = new DuplicateEntry(name, true, true);
        err->printError();
    }
    else {
        CustomType *ct = new Unknown();
        st.insert(name, ct, ENTRY_VARIABLE);
        this->type = ct;
    }
}

/************************************/
/*            PATTERN GEN           */
/************************************/

SymbolEntry *PatternGen::sem_getExprObj() { return pattern->sem_getExprObj(); }

void PatternGen::sem() {
    pattern->sem();
    this->type = pattern->getType();
    if (patternGen != nullptr) patternGen->sem();
}

/************************************/
/*          PATTERN CONSTR          */
/************************************/

SymbolEntry *PatternConstr::sem_getExprObj() { return st.lookup(Id); }

void PatternConstr::sem() {
    SymbolEntry *tempEntry = st.lookup(Id);
    if (tempEntry == nullptr) {
        /* Print Error - first Occurence */
        semError = true;
        this->type = new Unknown();
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
        Error *err = new FirstOccurence(Id);
        err->printError();
    }
    // else this->type = tempEntry->type;
    //CHANGE: make type CustomType instead of CustomId - TYPE_ID exists only in SymbolEntries
    else this->type = tempEntry->params.front()->type;
    if (patternGen != nullptr) {
        patternGen->sem();
        if (tempEntry != nullptr) {
            PatternGen *tempPatternGen = patternGen;
            int index = 0;
            SymbolEntry *patternEntry;
            while (tempPatternGen != nullptr) {

                /* get SymbolEntry of given param, if it's nullptr, means it's a PatternId, therefore do type inference */
                patternEntry = tempPatternGen->sem_getExprObj();
                if (patternEntry != nullptr && patternEntry->type->typeValue == TYPE_UNKNOWN) {
                    patternEntry->type = dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index);
                    if (patternEntry->type->typeValue == TYPE_FUNC) {
                        for(auto param : patternEntry->type->params)
                            patternEntry->params.push_back(new SymbolEntry(param));
                    }
                }
                // type checks
                // type check that it's the same base type in case it's another constructor
                if (tempPatternGen->getType()->typeValue == TYPE_CUSTOM && !tempPatternGen->sem_getExprObj()->params.empty()) {
                    if (tempPatternGen->sem_getExprObj()->params.front()->id != dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index)->name) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(tempEntry->params.front()->type, tempPatternGen->sem_getExprObj()->params.front()->type);
                        err->printError();
                    }
                }
                // type check for everything else
                else if (tempPatternGen->getType()->typeValue != TYPE_UNKNOWN && tempPatternGen->getType()->typeValue != dynamic_cast<CustomId *>(tempEntry->type)->getParams().at(index)->typeValue) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
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

/************************************/
/*              CLAUSE              */
/************************************/

SymbolEntry *Clause::sem_getExprObj() { return pattern->sem_getExprObj(); }

void Clause::sem() {
    pattern->sem();
    expr->sem();
    this->type = expr->getType();
}

/************************************/
/*           BAR CLAUSE GEN         */
/************************************/

void BarClauseGen::sem() {
    st.openScope();
    clause->sem();
    st.closeScope();
    this->type = clause->getType();
    if (barClauseGen != nullptr) barClauseGen->sem();
}

/************************************/
/*               MATCH              */
/************************************/

SymbolEntry *Match::sem_getExprObj() { return clause->getExpr()->sem_getExprObj(); }

void Match::sem() {
    st.openScope();
    expr->sem();
    st.openScope();
    clause->sem();
    st.closeScope();

    /* must get type of clause if clause does not call recursive function with no
    return type (aka itself), otherwise check next clause and setRecInfo then */
    if (isRec() && clause->getType() != nullptr && clause->getType()->typeValue != TYPE_UNKNOWN)
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                this->setRecInfo(false, "");
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }

    /* if type of expression is unknown, set it to type of clause */
    SymbolEntry *exprEntry = expr->sem_getExprObj();
    if (expr->getType()->typeValue == TYPE_UNKNOWN && exprEntry->type->typeValue == TYPE_UNKNOWN
        && clause->getPattern()->getType() != nullptr && clause->getPattern()->getType()->typeValue != TYPE_UNKNOWN) {
        exprEntry->type = clause->getPattern()->getType();
    }
    /* might need to remove below if and assign this->type even if clause doesn't have a type yet */
    if (clause->getType() != nullptr) {
        if (this->type != nullptr && this->type->typeValue == TYPE_UNKNOWN) {
            // Destroy the object but leave the space allocated.
            CustomType *tempCT = this->type;
            std::string ctName;
            if (!clause->getType()->name.empty()) ctName = clause->getType()->name;
            tempCT->~CustomType();

            // Create a new object in the same space.
            if (clause->getType()->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
            else if (clause->getType()->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
            else if (clause->getType()->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
            else if (clause->getType()->typeValue == TYPE_ARRAY && clause->getType()->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
            else if (clause->getType()->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
            else if (clause->getType()->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
            else if (clause->getType()->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
            else if (clause->getType()->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
        }
        else this->type = clause->getType();
    }

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
                if (prev->typeValue == TYPE_UNKNOWN && tempBarClauseGen->getType()->typeValue != TYPE_UNKNOWN) {
                    /* Change previous clause type according to current clause type */
                    if (prev->typeValue == TYPE_UNKNOWN) {
                        prev = tempBarClauseGen->getType();
                        if (isRec() && tempBarClauseGen->getType() != nullptr && tempBarClauseGen->getType()->typeValue != TYPE_UNKNOWN)
                            for (int index = recFunctions.size()-1; index >= 0; index++)
                                if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                                    this->setRecInfo(false, "");
                                    recFunctions.at(index)->type->outputType = this->type;
                                    recFunctions.erase(recFunctions.begin() + index);
                                    break;
                                }
                    }
                    /* Change previous clause SymbolEntry (if it exists) according to current clause type */
                    if (prevSE != nullptr && prevSE->type != nullptr) {
                        if (prevSE->type->typeValue == TYPE_UNKNOWN) {
                            prevSE->type = tempBarClauseGen->getType();
                        }
                    }
                }
                /* type inference for clause return type - if current is unknown */
                if (prev->typeValue != TYPE_UNKNOWN && tempBarClauseGen->getType()->typeValue == TYPE_UNKNOWN) {

                    // might need to implement destructor
                    tempBarClauseGen->setType(prev);

                    SymbolEntry *tempSE = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj();

                    /* in case that the clause return type is the function itself */
                    if (tempSE != nullptr)
                        if (tempSE->type->typeValue == TYPE_FUNC)
                            dynamic_cast<Function *>(tempSE->type)->outputType = prev;
                }

                /* type check all clause (need to be the same type) */
                if (prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                    /* if both are customtypes */
                    if (prev->typeValue == TYPE_CUSTOM && tempBarClauseGen->getType()->typeValue == TYPE_CUSTOM
                    && prev->name != tempBarClauseGen->getType()->name) {
                        /* Print Error - cannot unify a with b */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                        err->printError();
                    }
                    /* if prev is unrelated to types */
                    else if ((prev->typeValue != TYPE_ID && prev->typeValue != TYPE_CUSTOM) && prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                        /* Print Error - cannot unify a with b */
                        /* if current clause is a constructor while prev wasn't */
                        if (tempBarClauseGen->getType()->typeValue == TYPE_ID && tempBarClauseGen->getClause()->sem_getExprObj() != nullptr) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getClause()->sem_getExprObj()->params.front()->type);
                            err->printError();

                        }
                        /* if both clauses aren't constructors */
                        else {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                    }
                    /* if current is unrelated to types */
                    else if ((tempBarClauseGen->getType()->typeValue != TYPE_ID && tempBarClauseGen->getType()->typeValue != TYPE_CUSTOM) && prev->typeValue != tempBarClauseGen->getType()->typeValue) {
                        /* Print Error - cannot unify a with b */
                        /* if current clause is a constructor while prev wasn't */
                        if (prev->typeValue == TYPE_ID && prevSE != nullptr) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(dynamic_cast<CustomType *>(prevSE->params.front()->type), tempBarClauseGen->getType());
                            err->printError();

                        }
                        /* if both clauses aren't constructors */
                        else {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(prev, tempBarClauseGen->getType());
                            err->printError();
                        }
                    }
                }
            }
            /* in case that expr does not have TYPE_ID or TYPE_CUSTOM */
            else {

                if (exprEntry->type->typeValue == TYPE_ARRAY && exprEntry->type->ofType->typeValue == TYPE_UNKNOWN) {
                    exprEntry->type->ofType = clause->getPattern()->sem_getExprObj()->params.at(0)->type;
                }
                else if (((exprEntry->type->typeValue == TYPE_UNKNOWN && clausePatternType->typeValue != TYPE_UNKNOWN)
                    || (exprEntry->type->typeValue != TYPE_UNKNOWN && clausePatternType->typeValue == TYPE_CUSTOM))
                    && exprEntry->type->typeValue != TYPE_FUNC && exprEntry->type->typeValue != TYPE_ARRAY) {
                    exprEntry->type = clause->getPattern()->getType();
                    }
            }

            if (this->type != nullptr && this->type->typeValue == TYPE_UNKNOWN) {
                // Destroy the object but leave the space allocated.
                CustomType *tempCT = this->type;
                std::string ctName;
                if (!tempCT->name.empty()) ctName = prev->name;
                tempCT->~CustomType();

                // Create a new object in the same space.
                if (prev->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                else if (prev->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                else if (prev->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                else if (prev->typeValue == TYPE_ARRAY && prev->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                else if (prev->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                else if (prev->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                else if (prev->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                else if (prev->typeValue == TYPE_CUSTOM) { tempCT = new (tempCT) CustomType(); tempCT->name = ctName; }
            }
            else this->type = prev;

            /* move pointers */
            prev = tempBarClauseGen->getType();
            prevSE = tempBarClauseGen->getClause()->getExpr()->sem_getExprObj();
            tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
        }

        if (exprEntry->type->typeValue != TYPE_UNKNOWN) {

            /* type inference for first clause object */
            if (clause->sem_getExprObj() != nullptr && clause->sem_getExprObj()->type->typeValue == TYPE_UNKNOWN) {
                SymbolEntry *firstClauseObj = clause->sem_getExprObj();
                firstClauseObj->type = exprEntry->type;
            }

            /* if clause returns something other than Unknown or CustomType */
            if (clause->getPattern()->getType()->typeValue != TYPE_CUSTOM && clause->getPattern()->getType()->typeValue != TYPE_UNKNOWN) {
                /* if expr is something other than CustomType -> type mismatch CustomType(expr) with CustomType(clause) */
                if (exprEntry->type->typeValue == TYPE_CUSTOM) {
                    /* Print Error - cannot unify exprEntry->type->id with clause->getPattern()->getType()->typeValue */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                    err->printError();
                }
                else if (exprEntry->type->typeValue != clause->getPattern()->getType()->typeValue) {
                    /* Print Error - cannot unify exprEntry->type->typeValue with clause->getPattern()->getType()->typeValue */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getExpr()->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                    err->printError();
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
                    if (clausePatternType->typeValue != TYPE_CUSTOM && clausePatternType->typeValue != TYPE_UNKNOWN)
                        if (exprEntry->type->typeValue != clausePatternType->typeValue) {
                            /* Print Error - cannot unify a with b */
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(exprEntry->type, clausePatternType);
                            err->printError();
                        }
                }
                else if (clausePatternType->typeValue != TYPE_CUSTOM && clausePatternType->typeValue != TYPE_UNKNOWN) {
                    /* Print Error - cannot unify exprEntry->type->id with clause->getPattern()->getType()->typeValue */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_line << ", Characters " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.first_column << " - " << tempBarClauseGen->getClause()->getPattern()->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(exprEntry->type, clause->getPattern()->getType());
                    err->printError();
                }

                /* move pointers */
                tempBarClauseGen = tempBarClauseGen->getBarClauseGen();
                /* type inference for return type of clause */
                if (clauseType->typeValue == TYPE_UNKNOWN) clauseType = exprEntry->type;
            }
        }
    }
    st.closeScope();
}

/************************************/
/*                FOR               */
/************************************/

void For::sem() {
    /* Open scope, go to BinOp and save the variable */
    st.openScope();
    st.insert(id, new Integer(), ENTRY_VARIABLE);
    start->sem();
    /* type inference for start */
    if (start->getType()->typeValue == TYPE_UNKNOWN) {
        CustomType *startType = start->getType();
        startType->~CustomType();
        startType = new (startType) Integer();
    }
    /* type check for start */
    if (start->getType()->typeValue != TYPE_INT) {
        /* Print Error */
        std::vector<CustomType *> expectedType;
        expectedType.push_back(new Integer());
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << start->YYLTYPE.first_line << ", Characters " << start->YYLTYPE.first_column << " - " << start->YYLTYPE.last_column << std::endl;
        Error *err = new Expectation(expectedType, start->getType());
        err->printError();
    }
    end->sem();
    /* type inference for end */
    if (end->getType()->typeValue == TYPE_UNKNOWN) {
        CustomType *endType = end->getType();
        endType->~CustomType();
        endType = new (endType) Integer();
    }
    /* type check for end */
    if (end->getType()->typeValue != TYPE_INT) {
        /* Print Error */
        std::vector<CustomType *> expectedType;
        expectedType.push_back(new Integer());
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << end->YYLTYPE.first_line << ", Characters " << end->YYLTYPE.first_column << " - " << end->YYLTYPE.last_column << std::endl;
        Error *err = new Expectation(expectedType, end->getType());
        err->printError();
    }
    if (isRec()) {
        /* For type is expr type */
        expr->setRecInfo(true, this->getRecFuncName());
        this->setRecInfo(false, "");
    }
    /* if everything ok then proceed */
    expr->sem();
    this->type = expr->getType();
    st.closeScope();
}

/************************************/
/*              WHILE               */
/************************************/

void While::sem() {
    st.openScope();
    if (isRec()) {
        /* While type is expr type */
        expr->setRecInfo(true, this->getRecFuncName());
        this->setRecInfo(false, "");
    }

    loopCondition->sem();
    /* type inference for loopCondition */
    if (loopCondition->getType()->typeValue == TYPE_UNKNOWN) {
        CustomType *loopConditionType = loopCondition->getType();
        loopConditionType->~CustomType();
        loopConditionType = new (loopConditionType) Boolean();
    }
    /* type check for loopCondition */
    if (loopCondition->getType()->typeValue != TYPE_BOOL) {
        /* Print Error */
        semError = true;
        std::vector<CustomType *> expectedType;
        expectedType.push_back(new Boolean());
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << loopCondition->YYLTYPE.first_line << ", Characters " << loopCondition->YYLTYPE.first_column << " - " << loopCondition->YYLTYPE.last_column << std::endl;
        Error *err = new Expectation(expectedType, loopCondition->getType());
        err->printError();
    }
    expr->sem();
    this->type = expr->getType();
    st.closeScope();
}

/************************************/
/*                IF                */
/************************************/

void If::sem() {
    condition->sem();
    /* type inference */
    if (condition->getType()->typeValue == TYPE_UNKNOWN && condition->sem_getExprObj() != nullptr) {
        CustomType *ct = new Boolean();
        condition->setType(ct);
        CustomType *temp = condition->sem_getExprObj()->type;
        while (temp->typeValue == TYPE_REF) temp = temp->ofType;
        temp = ct;
    }

    /* type check */
    if (condition->getType()->typeValue != TYPE_BOOL) {
        /* print Error */
        semError = true;
        std::vector<CustomType *> expectedType;
        expectedType.push_back(new Boolean());
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << condition->YYLTYPE.first_line << ", Characters " << condition->YYLTYPE.first_column << " - " << condition->YYLTYPE.last_column << std::endl;
        Error *err = new Expectation(expectedType, condition->getType());
        err->printError();
    }

    /* type inference in case there is no else and
    outputType for rec function that might exist */
    if (expr2 == nullptr) {
        this->type = new Unit();

        if (isRec()) {
            for (int index = recFunctions.size()-1; index >= 0; index++)
                if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                    recFunctions.at(index)->type->outputType = this->type;
                    recFunctions.erase(recFunctions.begin() + index);
                    this->setRecInfo(false, "");
                    break;
                }

        }
    }

    expr1->sem();
    if (expr2 == nullptr) {
        if (expr1->getType()->typeValue == TYPE_UNKNOWN) {
            expr1->setType(new Unit());
        }
        else if (expr1->getType()->typeValue != TYPE_UNIT) {
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr1->YYLTYPE.first_line << ", Characters " << expr1->YYLTYPE.first_column << " - " << expr1->YYLTYPE.last_column << std::endl;
            Error *err = new TypeMismatch(expr1->getType(), new Unit());
            err->printError();
        }
    }
    /* if type hasn't been set as unit already, it needs to have expr1's type */
    if (this->type == nullptr || (this->type != nullptr && this->type->typeValue == TYPE_UNKNOWN)) this->type = expr1->getType();

    /* type inference for function in case expr1 calls function 
    with return type Unknown - might need to be deprecated */
    if (this->type->typeValue == TYPE_UNKNOWN && expr2 != nullptr) {
        expr2->setRecInfo(true, this->getRecFuncName());
        this->setRecInfo(false, "");
    }

    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }

    if (expr2 != nullptr) {
        expr2->sem();
        // if (this->type->typeValue == TYPE_UNKNOWN) this->type = expr2->getType();
        /* might need to reconsider expr1 and expr2 getType()->typeValue == TYPE_UNKNOWN */
        if (expr1->getType()->typeValue == TYPE_UNKNOWN) expr1->setType(expr2->getType());
        else if (expr2->getType()->typeValue == TYPE_UNKNOWN) expr2->setType(expr1->getType());
        /* expr1 and expr2 must be of same type */
        // else 
        if (expr1->getType()->typeValue != expr2->getType()->typeValue) {
            /* print Error */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
            Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
            err->printError();
        }
        else if (expr1->getType()->typeValue == TYPE_FUNC && expr2->getType()->typeValue == TYPE_FUNC) {
            // for params - might not be necessary
            // for (auto i = 0; i < dynamic_cast<Function *>(expr1->getType())->params.size(); ++i) {
            //     if (dynamic_cast<Function *>(expr1->getType())->params.at(i)->typeValue != dynamic_cast<Function *>(expr2->getType())->params.at(i)->typeValue) {} // print error?
            // }
            // type inference
            if (expr1->getType()->outputType->typeValue == TYPE_UNKNOWN && expr2->getType()->outputType->typeValue != TYPE_UNKNOWN) {
                CustomType *tempCT = expr1->getType()->outputType;
                CustomType *expr2outputType = expr2->getType()->outputType;
                tempCT->~CustomType();

                if (expr2outputType->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                else if (expr2outputType->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                else if (expr2outputType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                else if (expr2outputType->typeValue == TYPE_ARRAY && expr2outputType->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                else if (expr2outputType->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                else if (expr2outputType->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                else if (expr2outputType->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                else if (expr2outputType->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(expr2outputType->ofType);
            }
            if (expr2->getType()->outputType->typeValue == TYPE_UNKNOWN && expr1->getType()->outputType->typeValue != TYPE_UNKNOWN) {
                CustomType *tempCT = expr2->getType()->outputType;
                CustomType *expr1outputType = expr1->getType()->outputType;
                tempCT->~CustomType();

                if (expr1outputType->typeValue == TYPE_INT) tempCT = new (tempCT) Integer();
                else if (expr1outputType->typeValue == TYPE_FLOAT) tempCT = new (tempCT) Float();
                else if (expr1outputType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Character();
                else if (expr1outputType->typeValue == TYPE_ARRAY && expr1outputType->ofType->typeValue == TYPE_CHAR) tempCT = new (tempCT) Array(new Character(), 1);
                else if (expr1outputType->typeValue == TYPE_BOOL) tempCT = new (tempCT) Boolean();
                else if (expr1outputType->typeValue == TYPE_UNIT) tempCT = new (tempCT) Unit();
                else if (expr1outputType->typeValue == TYPE_UNKNOWN) tempCT = new (tempCT) Unknown();
                else if (expr1outputType->typeValue == TYPE_REF) tempCT = new (tempCT) Reference(expr1outputType->ofType);
            }

            // type check
            if (expr1->getType()->outputType->typeValue != expr2->getType()->outputType->typeValue) {
                /* print Error */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType()->outputType, expr2->getType()->outputType);
                err->printError();
            }
        }
    }
}

/************************************/
/*               BEGIN              */
/************************************/

void Begin::sem() {
    st.openScope();
    if (isRec()) {
        expr->setRecInfo(true, this->getRecFuncName());
        this->setRecInfo(false, this->getRecFuncName());
    }
    expr->sem();
    this->type = expr->getType();
    st.closeScope();
}

/************************************/
/*          COMMA EXPR GEN          */
/************************************/

void CommaExprGen::sem() {
    expr->sem();
    /* type inference */
    if (getRefFinalType(expr->getType()).first->typeValue == TYPE_UNKNOWN) {
        expr->setType(new Integer());
        SymbolEntry *se = expr->sem_getExprObj();
        if (se != nullptr) {
            CustomType *finalType = se->type;
            while(finalType->ofType != nullptr) finalType = finalType->ofType;
            finalType = new Integer();
        }
    }
    /* type check */
    if (expr->getType()->typeValue != TYPE_INT) {
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
        Error *err = new TypeMismatch(new Integer(), expr->getType());
        err->printError();
    }
    if (commaExprGen != nullptr) commaExprGen->sem();
}

/************************************/
/*               PAR                */
/************************************/

void Par::sem() {
    /* Params of a function are saved in a vector attribute (params) of the Symbol Entry of the function */
    SymbolEntry *tempEntry = st.getLastEntry();
    SymbolEntry *newParamEntry;
    if (type != nullptr) newParamEntry = new SymbolEntry(id, type);
    else newParamEntry = new SymbolEntry(id, new Unknown());
    if (newParamEntry->type->typeValue == TYPE_FUNC) {
        SymbolEntry *paramOfFunc;
        std::string name;
        int counter = 0;
        for (auto i : dynamic_cast<Function *>(newParamEntry->type)->params) {
            name = newParamEntry->id + "_param_" + std::to_string(counter++);
            paramOfFunc = new SymbolEntry(name, i);
            paramOfFunc->entryType = ENTRY_PARAMETER;
            newParamEntry->params.push_back(paramOfFunc);
        }
        reverse(newParamEntry->params.begin(), newParamEntry->params.end());
    }
    newParamEntry->entryType = ENTRY_PARAMETER;
    tempEntry->params.push_back(newParamEntry);
}

/************************************/
/*             PAR GEN              */
/************************************/

// int -> int -> int -> int
// Function(Function(int, Function(int, Function(int, nullptr))), int)
void ParGen::sem() {
    /* Params of a function are saved in a vector attribute (params) of the Symbol Entry of the function */
    // deprecated
    // SymbolEntry *tempEntry = st.getLastEntry();
    // if (parGen != nullptr) dynamic_cast<Function*>(tempEntry->type)->outputType = new Function(new Unknown(), new Unknown());
    par->sem();
    // deprecated
    // dynamic_cast<Function*>(tempEntry->type)->inputType = st.getLastEntry()->type;
    if (parGen != nullptr) parGen->sem();
}

/************************************/
/*                DEF               */
/************************************/

void Def::sem() {
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
            /* dimensions = -1 is temporary is updated in Let */
            /* array's type is given */
            if (type != nullptr) st.insert(id, new Array(type, -1), ENTRY_VARIABLE);
            /* array's type is unknown */
            else st.insert(id, new Array(new Unknown(), -1), ENTRY_VARIABLE);
        }
    }
    else {
        /* if def is a non mutable variable - constant */
        if (parGen == nullptr) {
            /* not null type */
            if (type != nullptr) {
                st.insert(id, type, ENTRY_CONSTANT);
                SymbolEntry *lastEntry = st.getLastEntry();
                if (lastEntry->type->typeValue == TYPE_FUNC) {
                    lastEntry->entryType = ENTRY_FUNCTION;
                    for (long unsigned int index = 0; index < lastEntry->type->params.size(); ++index) {
                        SymbolEntry *newParam = new SymbolEntry(id + "_param_" + std::to_string(index), lastEntry->type->params.at(index));
                        lastEntry->params.push_back(newParam);
                    }
                }
            }
            /* null type means that type is equal to expression's */
            else {
                st.insert(id, new Unknown(), ENTRY_CONSTANT);
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
                funcEntry->type->params.push_back(i->type);
                st.insert(i->id, i);
            }

            st.closeScope();
        }
    }
}

/************************************/
/*             DEF GEN              */
/************************************/

void DefGen::sem() {
    def->sem();
    if (defGen != nullptr) defGen->sem();
}

/************************************/
/*                LET               */
/************************************/

void Let::sem() {
    std::vector<SymbolEntry *> defsSE;
    SymbolEntry *tempSE;
    int index = 0;

    def->sem();
    defs.push_back(def);

    if (defGen != nullptr) defGen->sem();
    DefGen *tempDefGen = defGen;

    while (tempDefGen != nullptr) {
        defs.push_back(tempDefGen->getDef());
        tempDefGen = tempDefGen->getNext();
    }

    for (auto currDef : defs) {
        tempSE = st.lookup(currDef->id);
        defsSE.push_back(tempSE);
        if (!rec) tempSE->isVisible = false;
    }

    for (auto currDef : defs) {

        // st.printST();

        tempSE = defsSE.at(index++);

        /* if def is a mutable variable/array */
        if (currDef->mut) {
            /* variable */
            if (currDef->expr == nullptr) {}
            /* array */
            else {
                currDef->expr->sem();
                if (currDef->expr->getType()->typeValue != TYPE_INT) {
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << currDef->expr->YYLTYPE.first_line << ", Characters " << currDef->expr->YYLTYPE.first_column << " - " << currDef->expr->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(new Integer(), currDef->expr->getType());
                    err->printError();
                }
                if (currDef->commaExprGen != nullptr) currDef->commaExprGen->sem();

                /* get dimensions by iterating commaExprGen "list" */
                int dimensions = 1;
                CommaExprGen *tempExpr = currDef->commaExprGen;
                while (tempExpr != nullptr) {
                    dimensions++;
                    tempExpr = tempExpr->getNext();
                }

                tempSE->type->size = dimensions;
            }
        }
        else {
            /* if def is a non mutable variable - constant */
            if (currDef->parGen == nullptr) {
                currDef->expr->sem();
                /* not null type */
                if (currDef->type != nullptr) {
                    /* check if type given is not same as expression's */
                    if (currDef->type->typeValue != currDef->expr->getType()->typeValue) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << " Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(tempSE->type, currDef->expr->getType());
                        err->printError();
                    }
                    tempSE->type = currDef->type;
                }
                /* null type means that type is equal to expression's */
                else {

                    /* expression's type might be of type ref so we need to save the entry in st as entry variable and not constant (is mutable) */
                    tempSE->type = currDef->expr->getType();

                    if (currDef->expr->getType()->typeValue == TYPE_FUNC) {
                        SymbolEntry *paramOfFunc;
                        std::string name;
                        int counter = 0;
                        for (auto t : dynamic_cast<Function *>(currDef->expr->getType())->params) {
                            name = tempSE->id + "_param_" + std::to_string(counter++);
                            paramOfFunc = new SymbolEntry(name, t);
                            paramOfFunc->entryType = ENTRY_PARAMETER;
                            tempSE->params.push_back(paramOfFunc);
                        }
                    }

                    if (currDef->expr->getType()->typeValue != TYPE_REF) tempSE->entryType = ENTRY_CONSTANT;
                    else tempSE->entryType = ENTRY_VARIABLE;
                }
            }
            /* if def is a function */
            else {
                st.openScope();

                for (auto param : tempSE->params) {
                    st.insert(param->id, param);
                    // dynamic_cast<Function *>(tempSE->type)->params.push_back(param->type);
                }

                if(tempSE->isVisible) {
                    currDef->expr->setType(new Unknown());
                    dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                    currDef->expr->setRecInfo(true, tempSE->id);
                    recFunctions.push_back(tempSE);
                }
                currDef->expr->sem();

                long unsigned int counter = 0;
                while (counter < dynamic_cast<Function *>(tempSE->type)->params.size()) {
                    if (dynamic_cast<Function *>(tempSE->type)->params.at(counter)->typeValue == TYPE_UNKNOWN)
                        dynamic_cast<Function *>(tempSE->type)->params.at(counter) = tempSE->params.at(counter)->type;

                    counter++;
                }
                if (dynamic_cast<Function*>(tempSE->type)->outputType == nullptr ||
                (dynamic_cast<Function*>(tempSE->type)->outputType != nullptr && dynamic_cast<Function*>(tempSE->type)->outputType->typeValue == TYPE_UNKNOWN)) {
                    if (currDef->expr->sem_getExprObj() != nullptr) {
                        if (currDef->expr->sem_getExprObj()->type->typeValue == TYPE_FUNC) {
                            // if (currDef->expr->sem_getExprObj() != tempSE) dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type->outputType;
                            // else dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type->outputType;
                        }
                        else if (currDef->expr->getType()->typeValue == TYPE_CUSTOM)
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                        else if (currDef->expr->sem_getExprObj()->type->typeValue == TYPE_ARRAY)
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type->ofType;
                        else if (currDef->expr->sem_getExprObj()->type->typeValue == TYPE_REF)
                            dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                        else dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->sem_getExprObj()->type;
                    }
                    else dynamic_cast<Function*>(tempSE->type)->outputType = currDef->expr->getType();
                }

                st.closeScope();
            }
        }
    }

    for (auto se : defsSE) se->isVisible = true;

    // st.printST();
}

/************************************/
/*              LETIN               */
/************************************/

SymbolEntry *LetIn::sem_getExprObj() { return LetInSE; }

void LetIn::sem() {
    st.openScope();
    let->sem();
    for (auto se : st.lookup(let->def->id)->params) {
        se->isVisible = false;
    }
    if (isRec()) {
        expr->setRecInfo(true, this->getRecFuncName());
        this->setRecInfo(false, this->getRecFuncName());
    }
    expr->sem();
    LetInSE = expr->sem_getExprObj();
    this->type = expr->getType();
    st.closeScope();
}

/************************************/
/*              DELETE              */
/************************************/

void Delete::sem() {
    this->type = new Unit();
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
    expr->sem();
    if (expr->getType()->typeValue != TYPE_REF) {
        /* Print Error - type mismatch */
        semError = true;
        std::vector<CustomType *> expectedType;
        expectedType.push_back(new Reference(new Unknown()));
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
        Error *err = new Expectation(expectedType, expr->getType());
        err->printError();
    }
}

/************************************/
/*                NEW               */
/************************************/

void New::sem() {
    this->type = new Reference(type);
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*             ARRAYITEM            */
/************************************/

SymbolEntry *ArrayItem::sem_getExprObj() { return st.lookup(id); }

void ArrayItem::sem() {

    SymbolEntry *tempEntry = st.lookup(ENTRY_VARIABLE, id);
    if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_PARAMETER, id);
    if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_TEMP, id);
    if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_CONSTANT, id);
    if (tempEntry == nullptr) {
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
        Error *err = new FirstOccurence(id);
        err->printError();

        tempEntry = new SymbolEntry(id, new Unknown());
        /* Now Unknown is recognized as None */
        tempEntry->type->size = 0;
        this->type = tempEntry->type;
        tempEntry->entryType = ENTRY_TEMP;
        st.insert(id, tempEntry);
    }

    if (tempEntry->type->typeValue == TYPE_ARRAY && dynamic_cast<Array *>(tempEntry->type)->isInferred) dynamic_cast<Array *>(tempEntry->type)->isInferred = false;

    /* type inference */
    if (tempEntry->entryType != ENTRY_TEMP && tempEntry->type->typeValue == TYPE_UNKNOWN) {
        /* get dimensions by iterating commaExprGen "list" */
        int dims = 1;
        CommaExprGen *tempExpr = commaExprGen;
        while (tempExpr != nullptr) {
            dims++;
            tempExpr = tempExpr->getNext();
        }
        tempEntry->type = new Array(new Unknown(), dims);
    }

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
    if (expr->getType()->typeValue != TYPE_INT) {
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
        Error *err = new TypeMismatch(new Integer(), expr->getType());
        err->printError();
    }

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
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
        Error *err = new ArrayTypeMismatch(dimensions, new Array(new Unknown(), dimensions), tempEntry->type);
        err->printError();
    }
    else {
        if (dimensions == tempEntry->type->size) { /* all ok */ }
        else {
            /*  Print Error
                type mismatch in expression,
                a should be an array of 1 dimensions,
                impossible to unify int with @3
            */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new ArrayDimensions(new Array(new Unknown(), dimensions), dynamic_cast<Array *>(tempEntry->type));
            err->printError();
        }
    }
    if (tempEntry->type->typeValue != TYPE_UNKNOWN) this->type = tempEntry->type;
    else this->type = new Array(new Unknown(), dimensions);
    // might need to be moved up
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*                DIM               */
/************************************/

void Dim::sem() {
    this->type = new Integer();
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }

    SymbolEntry *tempEntry = st.lookup(ENTRY_VARIABLE, id);
    if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_PARAMETER, id);
    if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_TEMP, id);
    if (tempEntry == nullptr) tempEntry = st.lookup(ENTRY_CONSTANT, id);
    if (tempEntry == nullptr) {
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
        Error *err = new FirstOccurence(id);
        err->printError();

        tempEntry = new SymbolEntry(id, new Unknown());
        /* Now Unknown is recognized as None */
        tempEntry->type->size = 0;
        this->type = tempEntry->type;
        tempEntry->entryType = ENTRY_TEMP;
        st.insert(id, tempEntry);
    }

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
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new ArrayDimensions(new Array(new Unknown(), intconst), dynamic_cast<Array *>(tempEntry->type));
            err->printError();
        }
    }
    else {
        /* Print Error - Impossible to unify type given (symbolentry's type) with array */
        semError = true;
        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
        Error *err = new ArrayTypeMismatch(intconst, new Array(new Unknown(), intconst), tempEntry->type);
        err->printError();
    }
}

/************************************/
/*               BINOP              */
/************************************/

SymbolEntry *BinOp::sem_getExprObj() { if (!strcmp(op, ";")) return expr2->sem_getExprObj(); else return nullptr; }

void BinOp::sem() {
    
    if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "mod")) this->type = new Integer();
    else if (!strcmp(op, "+.") || !strcmp(op, "-.") || !strcmp(op, "*.") || !strcmp(op, "/.") || !strcmp(op, "**")) this->type = new Float();
    else if (!strcmp(op, "=") || !strcmp(op, "<>")) this->type = new Boolean();
    else if (!strcmp(op, "==") || !strcmp(op, "!=")) this->type = new Boolean();
    else if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, ">=") || !strcmp(op, "<=")) this->type = new Boolean();
    else if (!strcmp(op, "&&") || !strcmp(op, "||")) this->type = new Boolean();
    else if (!strcmp(op, ":=")) this->type = new Unit();
    /* ";" is the only op where rec functions get are moved to a deeper level of the AST */
    else if (!strcmp(op, ";")) {
        expr2->setRecInfo(true, getRecFuncName());
        this->setRecInfo(false, "");
    }

    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }

    expr1->sem();
    expr2->sem();

    SymbolEntry *tempExpr1 = expr1->sem_getExprObj();
    SymbolEntry *tempExpr2 = expr2->sem_getExprObj();

    /* type inference for all binops exept ';' & ':=' */
    if (strcmp(op, ";") && strcmp(op, ":=")) {

        /* both unknown */
        if (tempExpr1 != nullptr && tempExpr2 != nullptr
            && tempExpr1->entryType != ENTRY_TEMP && tempExpr2->entryType != ENTRY_TEMP
            && expr1->getType()->typeValue == TYPE_UNKNOWN && expr2->getType()->typeValue == TYPE_UNKNOWN) {
            expr1->setType(expr2->getType());
            // might need change for Ref(Ref(Ref(...)))
            if (tempExpr1->type->typeValue == TYPE_REF && tempExpr1->type->ofType->typeValue == TYPE_UNKNOWN && tempExpr2->type->typeValue == TYPE_UNKNOWN)
                tempExpr1->type->ofType = tempExpr2->type;
            else if (tempExpr2->type->typeValue == TYPE_REF && tempExpr2->type->ofType->typeValue == TYPE_UNKNOWN && tempExpr1->type->typeValue == TYPE_UNKNOWN)
                tempExpr1->type = tempExpr2->type->ofType;
            else
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
                else if (expr2->getType()->typeValue == TYPE_BOOL) {
                    expr1->getType()->~CustomType();
                    expr1->setType(new (expr1->getType()) Boolean());
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
                else if (expr1->getType()->typeValue == TYPE_BOOL) {
                    expr2->getType()->~CustomType();
                    expr2->setType(new (expr2->getType()) Boolean());
                }
                else { /* for now nothing */ }

                if (tempExpr2->type->typeValue != TYPE_FUNC) {
                    if (!(tempExpr2->type->typeValue == TYPE_ARRAY || tempExpr2->type->typeValue == TYPE_REF)) tempExpr2->type = expr2->getType();
                    else tempExpr2->type->ofType = expr2->getType();
                }
            }
        }
    }

    // set size of expr1->type to zero (Now is None instead of Unknown)
    if (tempExpr1 != nullptr && tempExpr1->entryType == ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) expr1->getType()->size = 0;

    // set size of expr2->type to zero (Now is None instead of Unknown)
    if (tempExpr2 != nullptr && tempExpr2->entryType == ENTRY_TEMP && expr2->getType()->typeValue == TYPE_UNKNOWN) expr2->getType()->size = 0;

    if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "mod")) {
        /* type inference - if both are unknown */
        if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
            // expr1->setType(new Integer());
            expr2->getType()->~CustomType();
            expr2->setType(new (expr2->getType()) Integer());
        }
        if (expr1->getType()->typeValue == TYPE_INT && expr2->getType()->typeValue == TYPE_INT && expr1->getType()->ofType == nullptr && expr2->getType()->ofType == nullptr) {}
        else {
            /* Print Error */
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Integer());

            Error *err;
            if (expr1->getType()->typeValue != TYPE_INT || expr1->getType()->ofType != nullptr) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr1->YYLTYPE.first_line - 1 << ", Characters " << expr1->YYLTYPE.first_column + 1 << " - " << expr1->YYLTYPE.last_column << std::endl;
                err = new Expectation(expectedType, expr1->getType());
                err->printError();
            }
            if (expr2->getType()->typeValue != TYPE_INT || expr2->getType()->ofType != nullptr) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column - 1 << " - " << expr2->YYLTYPE.last_column + 1 << std::endl;
                err = new Expectation(expectedType, expr2->getType());
                err->printError();
            }
        }
    }
    else if (!strcmp(op, "+.") || !strcmp(op, "-.") || !strcmp(op, "*.") || !strcmp(op, "/.") || !strcmp(op, "**")) {
        /* type inference - if both are unknown */
        if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
            // expr1->setType(new Float());
            expr2->getType()->~CustomType();
            expr2->setType(new (expr2->getType()) Float());
        }
        if (expr1->getType()->typeValue == TYPE_FLOAT && expr2->getType()->typeValue == TYPE_FLOAT) {}
        else {
            /* Print Error */
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Float());
            Error *err;
            if (expr1->getType()->typeValue != TYPE_FLOAT) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr1->YYLTYPE.first_line << ", Characters " << expr1->YYLTYPE.first_column << " - " << expr1->YYLTYPE.last_column << std::endl;
                err = new Expectation(expectedType, expr1->getType());
                err->printError();
            }
            if (expr2->getType()->typeValue != TYPE_FLOAT) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                err = new Expectation(expectedType, expr2->getType());
                err->printError();
            }
        }
    }
    else if (!strcmp(op, "=") || !strcmp(op, "<>")) {
        if (expr1->getType()->typeValue == expr2->getType()->typeValue && expr1->getType()->typeValue == TYPE_CUSTOM) {
            if (expr1->getType()->getName().compare(expr2->getType()->getName())) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (expr1->getType()->typeValue == expr2->getType()->typeValue && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {}
        else {
            /* Print Error */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
            err->printError();
        }
    }
    else if (!strcmp(op, "==") || !strcmp(op, "!=")) {
        /* type check */
        if (expr1->getType()->typeValue == expr2->getType()->typeValue && expr1->getType()->typeValue == TYPE_CUSTOM) {
            if (expr1->getType()->getName().compare(expr2->getType()->getName())) {
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
                err->printError();
            }
        }
        else if (expr1->getType()->typeValue == expr2->getType()->typeValue
            && expr1->getType()->typeValue != TYPE_ARRAY && expr1->getType()->typeValue != TYPE_FUNC) {}
        else {
            /* Print Error */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
            err->printError();
        }
    }
    else if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, ">=") || !strcmp(op, "<=")) {
        if (expr1->getType()->typeValue == expr2->getType()->typeValue
            && (expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT
            || expr1->getType()->typeValue == TYPE_CHAR || expr1->getType()->typeValue == TYPE_UNKNOWN)) {}
        else {
            /* Print Error */
            std::vector<CustomType *> expectedTypes;
            expectedTypes.push_back(new Integer());
            expectedTypes.push_back(new Float());
            expectedTypes.push_back(new Character());
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = nullptr;
            if (!(expr1->getType()->typeValue == TYPE_INT || expr1->getType()->typeValue == TYPE_FLOAT || expr1->getType()->typeValue == TYPE_CHAR)) err = new Expectation(expectedTypes, expr1->getType());
            if (!(expr2->getType()->typeValue == TYPE_INT || expr2->getType()->typeValue == TYPE_FLOAT || expr2->getType()->typeValue == TYPE_CHAR)) err = new Expectation(expectedTypes, expr2->getType());
            err->printError();
        }
    }
    else if (!strcmp(op, "&&") || !strcmp(op, "||")) {
        if (expr1->getType()->typeValue == expr2->getType()->typeValue && expr1->getType()->typeValue == TYPE_BOOL) {}
        else {
            /* Print Error */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
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
        if(expr2->getType()->typeValue == TYPE_ID) expr2->setType(expr2->getType()->params.front());

        SymbolEntry *tempEntry = expr1->sem_getExprObj();
        if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_ARRAY && expr1->getType()->ofType->typeValue == TYPE_UNKNOWN) {
            // if (expr2->getType()->typeValue != TYPE_ID) {
            //     expr1->getType()->ofType = expr2->getType();
            //     tempEntry->type->ofType = expr2->getType();
            // }
            // else {
            //     expr1->getType()->ofType = tempExpr2->params.front()->type;
            //     tempEntry->type->ofType = tempExpr2->params.front()->type;
            // }
            expr1->getType()->ofType = expr2->getType();
            tempEntry->type->ofType = expr2->getType();
        }
        else {
            if (tempExpr1 != nullptr && tempExpr1->entryType != ENTRY_TEMP && expr1->getType()->typeValue == TYPE_UNKNOWN) {
                if (tempExpr2 != nullptr) {
                    // expr1->setType(new Reference(expr2->getType()));
                    std::pair <CustomType *, int> pairExpr1, pairExpr2;
                    pairExpr1 = expr1->getRefFinalType(tempExpr1->type);
                    if(expr2->getType()->typeValue == TYPE_CUSTOM) {
                        if (tempExpr2->params.front()->type->typeValue != TYPE_FUNC)
                            pairExpr2 = expr2->getRefFinalType(tempExpr2->params.front()->type);
                        else pairExpr2 = expr2->getFnFinalType(tempExpr2->type);
                    }
                    else {
                        if (tempExpr2->type->typeValue != TYPE_FUNC)
                        pairExpr2 = expr2->getRefFinalType(tempExpr2->type);
                        else pairExpr2 = expr2->getFnFinalType(tempExpr2->type);
                    }
                    if (pairExpr1.first != pairExpr2.first) {
                        /* change SumbolEntry type */
                        CustomType *tempCT = tempExpr1->type;
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(pairExpr2.first);
                        /* Change expr type */
                        // std::cout <<"About to set tempexpr1 to ref of "; pairExpr2.first->printOn(std::cout); std::cout <<std::endl; std::cout.flush();
                        // std::cout <<"About to set expr1 to ref of "; expr2->getType()->printOn(std::cout); std::cout <<std::endl; std::cout.flush();
                        expr1->setType(new Reference(expr2->getType()));
                        
                    }
                    else {
                        recursiveRefError = true;
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(tempExpr1->type, tempExpr2->params.front()->type);
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
        }

        /* synchronize expr with tempExpr1 */
        if (expr1->getType()->typeValue == TYPE_REF && tempExpr1->type->typeValue == TYPE_UNKNOWN) {
            tempExpr1->type = expr1->getType();
        }
        else if (expr1->getType()->typeValue != TYPE_REF && tempExpr1->type->typeValue == TYPE_REF) {
            expr1->setType(tempExpr1->type);
        }

        if (tempEntry != nullptr && tempEntry->entryType != ENTRY_TEMP) {
            // if expr1 = Ref(Unknown) then replace Unknown with expr2 type
            if (expr1->getType()->typeValue == TYPE_REF && expr1->getType()->ofType->typeValue == TYPE_UNKNOWN) {
                /* edge case for when second operand points to first */
                if (expr2->getType()->typeValue == TYPE_REF
                && getRefFinalType(expr1->getType()).first == getRefFinalType(expr2->getType()).first) {
                    /* Print Error - type mismatch */
                    if (expr1->YYLTYPE.first_line == expr2->YYLTYPE.first_line) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << expr1->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    }
                    else {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Lines " << expr1->YYLTYPE.first_line << " - " << expr2->YYLTYPE.last_line << std::endl;
                    }

                    Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                    err->printError();
                }
                else {
                    if (getRefFinalType(expr2->getType()).first->typeValue == TYPE_ARRAY) {
                        if (getRefFinalType(expr2->getType()).first->ofType->typeValue == TYPE_CHAR && getRefFinalType(expr2->getType()).first->size == 1) { /* ref to string */ }
                        else {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(expr1->getType(), getRefFinalType(expr2->getType()).first);
                            err->printError();
                        }
                    }
                    expr1->getType()->ofType = expr2->getType();
                }
            }
            else if (expr1->getType()->typeValue == TYPE_ARRAY) {
                if (expr1->getType()->ofType->typeValue != expr2->getType()->typeValue) {
                    /* Print Error - type mismatch */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                    Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                    err->printError();
                }
            }
            // expr1 already has a ref type so need to compare type with expr2 type
            else {

                std::pair <CustomType *, int> pairExpr1, pairExpr2;

                if (expr1->getType()->typeValue == TYPE_REF) pairExpr1 = getRefFinalType(expr1->getType()->ofType);
                // if (expr1->getType()->typeValue == TYPE_REF) pairExpr1 = getRefFinalType(tempExpr1->type->ofType);
                if (expr2->getType()->typeValue == TYPE_REF) pairExpr2 = getRefFinalType(expr2->getType());

                if (!recursiveRefError) {
                    if (expr1->getType()->ofType->typeValue == expr2->getType()->typeValue) {

                        if (expr2->getType()->typeValue == TYPE_REF) {
                            if (pairExpr1.first->typeValue == pairExpr2.first->typeValue && pairExpr1.second == pairExpr2.second) {}
                            else {
                                /* Print Error */
                                semError = true;
                                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                                std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                                Error *err = new TypeMismatch(pairExpr1.first, pairExpr2.first);
                                err->printError();
                            }
                        }
                        else {}

                    }
                    else {
                        /* Print Error - type mismatch */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << expr2->YYLTYPE.first_line << ", Characters " << expr2->YYLTYPE.first_column << " - " << expr2->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(expr1->getType()->ofType, expr2->getType());
                        err->printError();
                    }
                }
            }
        }
        else {
            /* Print Error */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;


            // if needed, can check expr1 name as follows:
            // std::string str = expr1->getName();
            // if (str.length() == 0) {...}

            CustomType *typeOfExpr2 = expr2->getType();
            CustomType *tempRefType = new Reference(new Unknown());
            tempRefType->ofType = nullptr;
            typeOfExpr2->ofType = tempRefType;

            Error *err = new TypeMismatch(expr1->getType(), expr2->getType());
            err->printError();
        }
    }

}

/************************************/
/*                UNOP              */
/************************************/

SymbolEntry *UnOp::sem_getExprObj() { return expr->sem_getExprObj(); }

void UnOp::sem() {
    expr->sem();
    if (!strcmp(op, "!")) {
        /* If expr is Ref(type), make Dereference (convert Ref(type) to type) */
        /* or if expr is Array(type), make this type eq to array oftype */
        if (expr->getType()->typeValue == TYPE_REF || expr->getType()->typeValue == TYPE_ARRAY) {
            this->type = expr->getType()->ofType;
            // if (this->type->typeValue == TYPE_ID) this->type = this->type->params.front();
        }
        else {
            /* type inference */
            if (expr->getType()->typeValue == TYPE_UNKNOWN) {

                if (expr->sem_getExprObj() != nullptr) {
                    CustomType *tempCT = expr->sem_getExprObj()->type;

                    if (tempCT->typeValue == TYPE_UNIT) {
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(new Unit());
                    }
                    else if (tempCT->typeValue == TYPE_INT) {
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(new Integer());
                    }
                    else if (tempCT->typeValue == TYPE_CHAR) {
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(new Character());
                    }
                    else if (tempCT->typeValue == TYPE_BOOL) {
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(new Boolean());
                    }
                    else if (tempCT->typeValue == TYPE_FLOAT) {
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(new Float());
                    }
                    else if (tempCT->typeValue == TYPE_REF) {
                        std::pair <CustomType *, int> pairTempCT = getRefFinalType(tempCT);
                        CustomType *newFinalType = nullptr;

                        if (pairTempCT.first->typeValue == TYPE_UNIT) newFinalType = new Unit();
                        else if (pairTempCT.first->typeValue == TYPE_INT) newFinalType = new Integer();
                        else if (pairTempCT.first->typeValue == TYPE_CHAR) newFinalType = new Character();
                        else if (pairTempCT.first->typeValue == TYPE_BOOL) newFinalType = new Boolean();
                        else if (pairTempCT.first->typeValue == TYPE_FLOAT) newFinalType = new Float();
                        // else if (pairTempCT.first->typeValue == TYPE_ID) newFinalType = new Integer();
                        else if (pairTempCT.first->typeValue == TYPE_UNKNOWN) newFinalType = new Unknown();
                        else if (pairTempCT.first->typeValue == TYPE_CUSTOM) newFinalType = new CustomType();

                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(newFinalType);

                        pairTempCT.second--;
                        while (pairTempCT.second != 0) {
                            tempCT = expr->sem_getExprObj()->type;
                            expr->sem_getExprObj()->type = new Reference(tempCT);
                            pairTempCT.second--;
                        }
                    }
                    else if (tempCT->typeValue == TYPE_UNKNOWN) {
                        tempCT->~CustomType();
                        tempCT = new (tempCT) Reference(new Unknown());
                    }
                    else {
                        expr->sem_getExprObj()->type = new Reference(expr->sem_getExprObj()->type);
                    }

                    // expr->sem_getExprObj()->type = new Reference(expr->sem_getExprObj()->type);
                    expr->setType(new Reference(getRefFinalType(expr->sem_getExprObj()->type).first));
                }
                else expr->setType(new Reference(expr->getType()));
                this->type = expr->getType()->ofType;
            }
            /* If expr is not Ref(type) - reached the end of References - make an Invalid type Type(Ref(nullptr)) */
            else {
                CustomType *tempRef = new Reference(new Unknown());
                tempRef->ofType = nullptr;

                if (expr->sem_getExprObj() != nullptr) expr->sem_getExprObj()->type->ofType = tempRef;

                // this->type->ofType = new Reference(this->type->ofType);
                this->type = expr->sem_getExprObj()->type;
            }
        }
    }
    else if (!strcmp(op, "+")) {
        if (expr->getType()->typeValue != TYPE_INT) {
            /* Print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Integer());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = expr->getType();
    }
    else if (!strcmp(op, "-")) {
        if (expr->getType()->typeValue != TYPE_INT) {
            /* Print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Integer());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = expr->getType();
    }
    else if (!strcmp(op, "+.")) {
        if (expr->getType()->typeValue != TYPE_FLOAT) {
            /* Print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Float());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = expr->getType();
    }
    else if (!strcmp(op, "-.")) {
        if (expr->getType()->typeValue != TYPE_FLOAT) {
            /* Print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Float());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = expr->getType();
    }
    else if (!strcmp(op, "not")) {
        if (expr->getType()->typeValue != TYPE_BOOL) {
            /* Print Error */
            semError = true;
            std::vector<CustomType *> expectedType;
            expectedType.push_back(new Boolean());
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << expr->YYLTYPE.first_line << ", Characters " << expr->YYLTYPE.first_column << " - " << expr->YYLTYPE.last_column << std::endl;
            Error *err = new Expectation(expectedType, expr->getType());
            err->printError();
        }
        this->type = expr->getType();
    }
    else { /* Left for debugging */ }
    // might need to be moved up
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*             INTCONST             */
/************************************/

void IntConst::sem() { 
    this->type = new Integer(); 
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*            FLOATCONST            */
/************************************/

void FloatConst::sem() { 
    this->type = new Float(); 
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*             CHARCONST            */
/************************************/

void CharConst::sem() { 
    this->type = new Character();
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*          STRINGLITERAL           */
/************************************/

void StringLiteral::sem() { 
    this->type = new Array(new Character(), 1);
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*           BOOLEANCONST           */
/************************************/

void BooleanConst::sem() {
    this->type = new Boolean();
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*            UNITCONST             */
/************************************/

void UnitConst::sem() {
    this->type = new Unit();
    if (isRec())
        for (int index = recFunctions.size()-1; index >= 0; index++)
            if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                recFunctions.at(index)->type->outputType = this->type;
                recFunctions.erase(recFunctions.begin() + index);
                break;
            }
}

/************************************/
/*             TYPEGEN              */
/************************************/

void TypeGen::sem() {
    if (typeGen != nullptr) typeGen->sem();
}

/************************************/
/*              CONSTR              */
/************************************/

SymbolEntry *Constr::sem_getExprObj() { return st.lookup(Id); }

void Constr::sem() {
    /* Constructor (Id) is called therefore need to give this Expr class the SymbolEntry's type and type check its params */
    if (call) {
        SymbolEntry *tempEntry = st.lookup(Id);
        if (tempEntry != nullptr) {
            /* Type check first param */
            if (expr != nullptr) {
                expr->sem();
                /* type inference */
                if (expr->getType()->typeValue == TYPE_UNKNOWN && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0)->typeValue != TYPE_UNKNOWN) {
                    SymbolEntry *tempParam = expr->sem_getExprObj();
                    if (tempParam->type->typeValue == TYPE_FUNC && dynamic_cast<Function*>(tempParam->type)->outputType != nullptr && dynamic_cast<Function*>(tempParam->type)->outputType->typeValue == TYPE_UNKNOWN) {
                        dynamic_cast<Function*>(tempParam->type)->outputType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                        expr->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0));
                    }
                    else if (tempParam->type->typeValue == TYPE_REF && tempParam->type->ofType->typeValue == TYPE_UNKNOWN) {
                        tempParam->type->ofType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                        expr->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0));
                    }
                    else {
                        tempParam->type = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0);
                        expr->setType(tempParam->type);
                    }
                }
                /* type check */
                if (expr->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(0)->typeValue) {
                    SymbolEntry *se = nullptr;
                    if (expr->getType()->typeValue == TYPE_ID) se = st.lookup(expr->getType()->name);
                    if (se != nullptr && expr->getType()->typeValue == TYPE_ID && se->type->typeValue == TYPE_ID && dynamic_cast<CustomId*>(tempEntry->type)->getParams().front()->typeValue == TYPE_CUSTOM
                    && se->params.front()->id != dynamic_cast<CustomId*>(tempEntry->type)->getParams().front()->name) {
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new TypeMismatch(expr->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().front());
                        err->printError();
                    }
                }
            }
            /* Type check the rest of the params */
            if (exprGen != nullptr) {
                exprGen->sem();
                ExprGen* tempExprGen = exprGen;
                int i = 1;
                while (tempExprGen != nullptr) {
                    /* type inference */
                    if (tempExprGen->getExpr()->getType()->typeValue == TYPE_UNKNOWN && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue != TYPE_UNKNOWN) {
                        SymbolEntry *tempParam = tempExprGen->getExpr()->sem_getExprObj();
                        if (tempParam->type->typeValue == TYPE_FUNC && dynamic_cast<Function*>(tempParam->type)->outputType != nullptr && dynamic_cast<Function*>(tempParam->type)->outputType->typeValue == TYPE_UNKNOWN) {
                            dynamic_cast<Function*>(tempParam->type)->outputType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i);
                            tempExprGen->getExpr()->setType(dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                        }
                        else if (tempParam->type->typeValue == TYPE_REF && tempParam->type->ofType->typeValue == TYPE_UNKNOWN) {
                            tempParam->type->ofType = dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i);
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
                        if (tempExprGen->getExpr()->getType()->typeValue == TYPE_ID && tempExprGen->getExpr()->sem_getExprObj()->type->typeValue == TYPE_ID && dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue == TYPE_CUSTOM
                        && tempExprGen->getExpr()->sem_getExprObj()->params.front()->type->name != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->name) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << tempExprGen->getExpr()->YYLTYPE.first_column << " - " << tempExprGen->getExpr()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempExprGen->getExpr()->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                            err->printError();
                        }
                        /* if it's a different type */
                        if ((tempExprGen->getExpr()->getType()->typeValue != TYPE_ID || tempExprGen->getExpr()->sem_getExprObj()->type->typeValue != TYPE_ID || dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue != TYPE_CUSTOM)
                        && tempExprGen->getExpr()->getType()->typeValue != dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i)->typeValue) {
                            semError = true;
                            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                            std::cout << "Error at: Line " << tempExprGen->getExpr()->YYLTYPE.first_line << ", Characters " << tempExprGen->getExpr()->YYLTYPE.first_column << " - " << tempExprGen->getExpr()->YYLTYPE.last_column << std::endl;
                            Error *err = new TypeMismatch(tempExprGen->getExpr()->getType(), dynamic_cast<CustomId*>(tempEntry->type)->getParams().at(i));
                            err->printError();
                        }
                    }
                    i++;
                    tempExprGen = tempExprGen->getNext();
                }
            }
            // this->type = tempEntry->type;
            // this->type->params.push_back(tempEntry->params.front()->type);
            //CHANGE: return CustomType instead of CustomId - TYPE_ID exists only in SymbolEntries
            this->type = tempEntry->params.front()->type;
            if (isRec())
                for (int index = recFunctions.size()-1; index >= 0; index++)
                    if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                        recFunctions.at(index)->type->outputType = this->type;
                        recFunctions.erase(recFunctions.begin() + index);
                        break;
                    }
        }
        else {
            this->type = new Unknown();
            /* Print Error - Id not exist in st */
            semError = true;
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
            Error *err = new FirstOccurence(Id);
            err->printError();
        }
    }
    else {
        CustomType *ct = new CustomId(Id);
        this->type = ct;
        if (isRec())
            for (int index = recFunctions.size()-1; index >= 0; index++)
                if (!getRecFuncName().compare(recFunctions.at(index)->id)) {
                    recFunctions.at(index)->type->outputType = this->type;
                    recFunctions.erase(recFunctions.begin() + index);
                    break;
                }
        if (typeGen != nullptr) {
            typeGen->sem();

            TypeGen *tempTypeGen = typeGen;
            while (tempTypeGen != nullptr) {
                if (typeGen->type->typeValue == TYPE_ID && !st.lookup(typeGen->type->name)) {
                    /* Print Error */
                    semError = true;
                    if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                    std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                    Error *err = new Error(typeGen->type->name + " doesn't exist in ST");
                    err->printMessage();
                    exit(1);
                }
                dynamic_cast<CustomId*>(ct)->pushToParams(tempTypeGen->type);
                tempTypeGen = tempTypeGen->typeGen;
            }
        }

        st.insert(Id, ct, ENTRY_CONSTRUCTOR);
    }
}

/************************************/
/*                TDEF              */
/************************************/

void Tdef::sem() {
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
        // bool isDuplicate = false;
        for (auto p : typeEntry->params) {
            if (p->id == tempConstr->id) {
                /* Print Error - duplicate type color = Red | Red | Blue | Yellow */
                semError = true;
                if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                Error *err = new DuplicateEntry(tempConstr->id, false);
                err->printError();
                // isDuplicate = true;
            }
        }
        // if (!isDuplicate) {
            /* Constructors of a user defined type */
            typeEntry->params.push_back(tempConstr);
            /* User defined type in a Constructor (single argument) */
            tempConstr->params.push_back(typeEntry);
        // }
        tempBarConstrGen = tempBarConstrGen->getNext();
    }
}

/************************************/
/*              TDEFGEN             */
/************************************/

void TdefGen::sem() {
    tDef->sem();
    if (tDefGen != nullptr) tDefGen->sem();
}

/************************************/
/*              TYPEDEF             */
/************************************/

void TypeDef::sem() {
    tDef->sem();
    if (tDefGen != nullptr) tDefGen->sem();
    /* search CustomId params in Constr in types */
    SymbolEntry *tempEntry = st.lookup(tDef->getName());
    /* for every Constr object in user defined type */
    for (auto constr : tempEntry->params) {
        /* if its *type* params (Constr of {params}) are not empty */
        if (!dynamic_cast<CustomId*>(constr->type)->getParams().empty()) {
            int i = 0;
            /* for every *type* param */
            for (auto typeParam : dynamic_cast<CustomId*>(constr->type)->getParams()) {
                /* if current *type* param is CustomId, make it point to user defined type, else it's not defined */
                if (typeParam->typeValue == TYPE_ID) {
                    SymbolEntry *typeEntry = st.lookup(typeParam->name);
                    if (typeEntry != nullptr) dynamic_cast<CustomId*>(constr->type)->replaceParam(typeEntry->type, i);
                    else {
                        /* Print Error - unbound constructor */
                        semError = true;
                        if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
                        std::cout << "Error at: Line " << this->YYLTYPE.first_line << ", Characters " << this->YYLTYPE.first_column << " - " << this->YYLTYPE.last_column << std::endl;
                        Error *err = new Error("Unbound constructor");
                        err->printMessage();
                    }
                }
                i++;
            }
        }
    }
}