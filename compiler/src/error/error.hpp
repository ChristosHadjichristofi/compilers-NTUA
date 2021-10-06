#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include "../types/types.hpp"
#include <vector>

class Error {
public:
    Error();
    Error(std::string cm);
    virtual void printError();
    void printMessage();

protected:
    std::string customMessage;
};

class TypeMismatch : public Error {
public:
    TypeMismatch();
    TypeMismatch(CustomType *ft, CustomType *st);
    virtual void printError() override;

protected:
    CustomType *firstType;
    CustomType *secondType;
};

class ArrayTypeMismatch : public TypeMismatch {
public:
    ArrayTypeMismatch(int ad, CustomType *ft, CustomType *st);
    virtual void printError() override;

protected:
    int arrayDimensions;
    CustomType *firstType;
    CustomType *secondType;
};

class ArrayDimensions : public Error {
public:
    ArrayDimensions(Array *ga, Array *ae);
    virtual void printError() override;

protected:
    Array *givenArray;
    Array *arrayExpected;
};

class FirstOccurence : public Error {
public:
    FirstOccurence(std::string id);
    virtual void printError() override;

protected:
    std::string id;
};

class Expectation : public Error {
public:
    Expectation(std::vector<CustomType *> et, CustomType *gt);
    virtual void printError() override;

private:
    std::vector<CustomType *> expectedTypes;
    CustomType *givenType;
};

class DuplicateEntry : public Error {
public:
    DuplicateEntry(std::string id, bool it, bool isNT = false);
    virtual void printError() override;

protected:
    std::string id;
    bool isType;
    bool isNotType;
};

class Warning : public Error {
public:
    Warning(std::string id);
    virtual void printError() override;

protected:
    std::string id;

};
#endif