#include "error.hpp"

/************************************/
/*               ERROR              */
/************************************/

Error::Error() {}

Error::Error(std::string cm): customMessage(cm) {}

void Error::printError() {}

void Error::printMessage() {
    std::cout << customMessage << std::endl;
}

/************************************/
/*           TYPEMISMATCH           */
/************************************/

TypeMismatch::TypeMismatch() {}

TypeMismatch::TypeMismatch(CustomType *ft, CustomType *st): firstType(ft), secondType(st) {}

void TypeMismatch::printError() {
    std::cout << "\tType mismatch: impossible to unify "; // on line ...
    firstType->printOn(std::cout);
    std::cout << " with ";
    secondType->printOn(std::cout);
    std::cout << "." << std::endl;
}

/************************************/
/*        ARRAYTYPEMISMATCH         */
/************************************/

ArrayTypeMismatch::ArrayTypeMismatch(int ad, CustomType *ft, CustomType *st): arrayDimensions(ad), firstType(ft), secondType(st) {}

void ArrayTypeMismatch::printError() {
    // on line
    std::cout << "\tType mismatch: should be an array of " << arrayDimensions << " dimensions. " << std::endl;
    std::cout << "\tImpossible to unify ";
    firstType->printOn(std::cout);
    std::cout << " with ";
    secondType->printOn(std::cout);
    std::cout << "." << std::endl;
}

/************************************/
/*         ARRAYDIMENSIONS          */
/************************************/

ArrayDimensions::ArrayDimensions(Array *ga, Array *ae): givenArray(ga), arrayExpected(ae) {}

void ArrayDimensions::printError() {
    // on line
    std::cout << "\tType mismatch: should be an array of " << arrayExpected->size << " dimensions, but " << givenArray->size << " were given.\n";
    std::cout << "\tImpossible to unify array [*";
    for(int i = 1; i < arrayExpected->size; i++)
        std::cout << ", *";
    std::cout << "] of ";
    arrayExpected->ofType->printOn(std::cout);
    std::cout << " with array [*";
    for(int i = 1; i < givenArray->size; i++)
        std::cout << ", *";
    std::cout << "] of ";
    givenArray->ofType->printOn(std::cout);
    std::cout << "." << std::endl;
}

/************************************/
/*          FIRSTOCCURANCE          */
/************************************/

FirstOccurence::FirstOccurence(std::string id): id(id) {}

void FirstOccurence::printError() {
    std::cout << "\tFirst occurence of identifier " << id << "." << std::endl; // on line ...
}

/************************************/
/*            EXPECTATION           */
/************************************/

Expectation::Expectation(std::vector<CustomType *> et, CustomType *gt): expectedTypes(et), givenType(gt) {}

void Expectation::printError() {
    // on line
    if(expectedTypes.size() > 1){
        std::cout << "\tExpected one of types ";
        bool firstType = true;
        for(auto t : expectedTypes){
            if (!firstType) std::cout <<", ";
            else firstType = false;
            t->printOn(std::cout);
        }
    }
    else {
        std::cout << "\tExpected type ";
        expectedTypes.front()->printOn(std::cout);
    }
    std::cout << " but got ";
    givenType->printOn(std::cout);
    std::cout << " instead." << std::endl;
}

/************************************/
/*          DUPLICATEENTRY          */
/************************************/

DuplicateEntry::DuplicateEntry(std::string id, bool it, bool isNT): id(id), isType(it), isNotType(isNT) {}

void DuplicateEntry::printError() {
    if(!isNotType) {
        if(isType) std::cout << "\tDuplicate declaration of type " << id << " given." << std::endl;
        else std::cout << "\tDuplicate declaration of constructor " << id << " given." << std::endl;
    }
    else std::cout << "\tDuplicate declaration of " << id << "." << std::endl;
}

/************************************/
/*              WARNING             */
/************************************/

Warning::Warning(std::string id): id(id) {}

void Warning::printError() {
    std::cout << "\tUnused polymorphic value, type for variable " << id << " not substituted." << std::endl;
}