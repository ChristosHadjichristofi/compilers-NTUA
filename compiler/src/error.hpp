#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include "types.hpp"
#include <vector>

class Error {
public:
    Error() {}
    Error(std::string cm): customMessage(cm) {}
    
    virtual void printError() {}
    
    void printMessage() {
        std::cout << customMessage <<std::endl;
    }

protected:
    std::string customMessage;
};

class TypeMismatch : public Error {
public:
    TypeMismatch() {}
    TypeMismatch(CustomType *ft, CustomType *st): firstType(ft), secondType(st) {}

    virtual void printError() override {
        std::cout << "Type mismatch: impossible to unify "; // on line ...
        firstType->printOn(std::cout);
        std::cout << " with ";
        secondType->printOn(std::cout);
        std::cout << "." << std::endl;
    }

protected:
    CustomType *firstType;
    CustomType *secondType;
};

class ArrayTypeMismatch : public TypeMismatch {
public:
    ArrayTypeMismatch(int ad, CustomType *ft, CustomType *st): arrayDimensions(ad), firstType(ft), secondType(st) {}

    virtual void printError() override {
        // on line
        std::cout << "Type mismatch: should be an array of " << arrayDimensions << " dimensions. ";
        std::cout << "Impossible to unify ";
        firstType->printOn(std::cout);
        std::cout << " with ";
        secondType->printOn(std::cout);
        std::cout << "." << std::endl;
    }

protected:
int arrayDimensions;
CustomType *firstType;
CustomType *secondType;
};

class ArrayDimensions : public Error {
public:
    ArrayDimensions(Array *ga, Array *ae): givenArray(ga), arrayExpected(ae) {}

    virtual void printError() override {
        // on line
        std::cout << "Type mismatch: should be an array of " << arrayExpected->size << " dimensions, but " << givenArray->size <<" were given. ";
        std::cout << "Impossible to unify array [*";
        for(int i = 1; i < arrayExpected->size; i++)
            std::cout <<", *";
        std::cout << "] of ";
        arrayExpected->ofType->printOn(std::cout);
        std::cout << " with array [*";
        for(int i = 1; i < givenArray->size; i++)
            std::cout <<", *";
        std::cout << "] of ";
        givenArray->ofType->printOn(std::cout);
        std::cout << "." << std::endl;
    }

protected:
    Array *givenArray;
    Array *arrayExpected;
};

class FirstOccurence : public Error {
public:
    FirstOccurence(std::string id): id(id) {}

    virtual void printError() override {
        std::cout << "First occurence of identifier " << id << "." << std::endl; // on line ...
    }

protected:
    std::string id;
};

class Expectation : public Error {
public:
    Expectation(std::vector<CustomType *> et, CustomType *gt): expectedTypes(et), givenType(gt) {}

    virtual void printError() override {
        // on line
        if(expectedTypes.size() > 1){
            std::cout << "Expected one of types ";
            bool firstType = true;
            for(auto t : expectedTypes){
                if (!firstType) std::cout <<"/";
                else firstType = false;
                t->printOn(std::cout);
            }
        }
        else {
            std::cout << "Expected type ";
            expectedTypes.front()->printOn(std::cout);
        }
        std::cout << " but got ";
        givenType->printOn(std::cout);
        std::cout << " instead." << std::endl;
    }

private:
std::vector<CustomType *> expectedTypes;
CustomType *givenType;
};

class DuplicateEntry : public Error {
public:
    DuplicateEntry(std::string id, bool it): id(id), isType(it) {}

    virtual void printError() override {
        if(isType) {
            std::cout << "Duplicate declaration of type " <<id <<" given." << std::endl;
        }
        else {
            std::cout << "Duplicate declaration of constructor " <<id <<" given." << std::endl;
        }
    }

protected:
std::string id;
bool isType;
};






#endif