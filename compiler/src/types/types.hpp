#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include "../ast/AST.hpp"
#include "../lexer/lexer.hpp"
#include <string>

enum Types { TYPE_UNIT, TYPE_INT, TYPE_CHAR, TYPE_BOOL, TYPE_FLOAT, TYPE_FUNC, TYPE_REF, TYPE_ARRAY, TYPE_ID, TYPE_UNKNOWN, TYPE_CUSTOM };

/****************************************************/
/*                        INFO                      */
/****************************************************/
/* All printOn functions are in ast/printOn.cpp     */
/* All compile functions are in ast/compile.cpp     */
/* getLLVMType is in ast/compile.cpp                */
/****************************************************/

class CustomType : public AST {
public:
   CustomType();
   CustomType(std::string n);
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName();
   virtual llvm::Value* compile() override;
   llvm::Type* getLLVMType();

   Types typeValue = TYPE_CUSTOM;
   std::vector<CustomType *> params;
   CustomType *outputType;
   CustomType *ofType;
   int size;
   std::string name;
};

class Unit : public CustomType {
public:
   Unit();
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

class Integer : public CustomType {
public:
   Integer();
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

class Character : public CustomType {
public:
   Character();
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

class Boolean : public CustomType {
public:   
   Boolean();
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

class Float : public CustomType {
public:
   Float();
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

class Function : public CustomType {
public:
   Function(/*CustomType *it ,*/ CustomType *ot);
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

class Reference : public CustomType {
public:
   Reference(CustomType *ct);
   virtual std::string getName() override;
   virtual void printOn(std::ostream &out) const override;
   virtual llvm::Value* compile() override;

};

class Array : public CustomType {
public:
   Array(CustomType *ct, int s);
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;
   
   bool isInferred;
};

class CustomId : public CustomType {
public:
   CustomId(std::string n);
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual std::vector<CustomType *> getParams();
   virtual void pushToParams(CustomType *newParam);
   virtual void replaceParam(CustomType *newType, int i);
   virtual llvm::Value* compile() override;
 
private:
   std::vector<CustomType *> params;
};

class Unknown : public CustomType {
public:
   Unknown();   
   virtual void printOn(std::ostream &out) const override;
   virtual std::string getName() override;
   virtual llvm::Value* compile() override;

};

#endif