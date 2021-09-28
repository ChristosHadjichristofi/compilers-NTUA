#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include "../ast/AST.hpp"
#include "../lexer/lexer.hpp"
#include <string>

enum Types { TYPE_UNIT, TYPE_INT, TYPE_CHAR, TYPE_BOOL, TYPE_FLOAT, TYPE_FUNC, TYPE_REF, TYPE_ARRAY, TYPE_ID, TYPE_UNKNOWN, TYPE_CUSTOM };

class CustomType : public AST {
public:
   CustomType() {}

   CustomType(std::string n): name(n) {}

   virtual void printOn(std::ostream &out) const override {
      if (!name.empty()) out << "CustomType(" << name << ")";
      else out << "CustomType()";
   }

   virtual std::string getName() { return name; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

   llvm::Type* getLLVMType() {
      if (typeValue == TYPE_INT) return i32;
      if (typeValue == TYPE_FLOAT) return DoubleTyID;
      if (typeValue == TYPE_BOOL) return i1;
      if (typeValue == TYPE_ARRAY && ofType != nullptr && ofType->typeValue == TYPE_CHAR) return TheModule->getTypeByName("Array_String_1");
      if (typeValue == TYPE_ARRAY) {
         /* name of struct type that we're searching */
         std::string arrName = "Array_" + ofType->getName() + "_" + std::to_string(size);

         if (TheModule->getTypeByName(arrName) != nullptr) return TheModule->getTypeByName(arrName)->getPointerTo();

         /* create array */
         std::vector<llvm::Type *> members;
         /* ptr to array */
         members.push_back(llvm::PointerType::getUnqual(ofType->getLLVMType()));
         /* dimensions number of array */
         members.push_back(i32);

         for (int i = 0; i < size; i++) members.push_back(i32);         

         /* create the struct */
         llvm::StructType *arrayStruct = llvm::StructType::create(TheContext, arrName);
         arrayStruct->setBody(members);

         return arrayStruct->getPointerTo();
      }
      if (typeValue == TYPE_CHAR) return i8;
      if (typeValue == TYPE_REF) return ofType->getLLVMType();
      if (typeValue == TYPE_UNKNOWN) return TheModule->getTypeByName("unit"); 
      if (typeValue == TYPE_UNIT) return TheModule->getTypeByName("unit");

      return nullptr;
   }

   llvm::Value * getLLVMValue() {
      if (typeValue == TYPE_INT) return c32(0);
      if (typeValue == TYPE_FLOAT) return fp(0);
      if (typeValue == TYPE_BOOL) return c1(0);
      if (typeValue == TYPE_CHAR) return c8(0);
      if (typeValue == TYPE_REF || typeValue == TYPE_ARRAY) return ofType->getLLVMValue();
      if (typeValue == TYPE_UNIT) return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
   
      return nullptr;
   }

   Types typeValue = TYPE_CUSTOM;
   std::vector<CustomType *> params;
   CustomType *outputType;
   CustomType *ofType;
   int size;
   std::string name;
};

class Unit : public CustomType {
public:
   Unit() { typeValue = TYPE_UNIT; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Unit"; 
      if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
   }

   virtual std::string getName() override { return "Unit"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Integer : public CustomType {
public:
   Integer() { typeValue = TYPE_INT; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Integer"; 
      if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
   }

   virtual std::string getName() override { return "Integer"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Character : public CustomType {
public:
   Character() { typeValue = TYPE_CHAR; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Character"; 
      if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
   }

   virtual std::string getName() override { return "Character"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Boolean : public CustomType {
public:   
   Boolean() { typeValue = TYPE_BOOL; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Boolean"; 
      if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
   }

   virtual std::string getName() override { return "Boolean"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Float : public CustomType {
public:
   Float() { typeValue = TYPE_FLOAT; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Float"; 
      if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
   }

   virtual std::string getName() override { return "Float"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Function : public CustomType {
public:
   Function(/*CustomType *it ,*/ CustomType *ot) { outputType = ot; typeValue = TYPE_FUNC; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "fn: ";
      if (!params.empty()) 
         for (auto i : params) { i->printOn(out); out << " -> "; } 
      outputType->printOn(out);
   }

   virtual std::string getName() override { return "Function"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Reference : public CustomType {
public:
   Reference(CustomType *ct) { 
      typeValue = TYPE_REF; 
      // this should be for keyword 'new'
      if(ct->typeValue == TYPE_ARRAY){
         yyerror("Input cannot be of CustomType 'Array'");
      }
      ofType = ct;
      size = -1; 
   }

   virtual std::string getName() override { return "Reference"; }

   virtual void printOn(std::ostream &out) const override {
      out << "Reference"; 
      if(ofType != nullptr) {
         out <<" of type "; ofType->printOn(out); if (SHOW_MEM) out << "  MEM:  " << ofType;
      }
   }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

class Array : public CustomType {
public:
   Array(CustomType *ct, int s) {
      typeValue = TYPE_ARRAY;
      if(ct->typeValue == TYPE_ARRAY){
         yyerror("Input cannot be of CustomType 'Array'");
      }
      ofType = ct;
      size = s;
      isInferred = false;
   }

   virtual void printOn(std::ostream &out) const override {
      // out << "Array(ofType"; ofType->printOn(out); out <<", size:" << size <<")";
      out << "Array [";
      for (int i = 0; i < size - 1; i++) out << "*,";
      out << "*] of type "; 
      ofType->printOn(out);
   }

   virtual std::string getName() override { return "Array"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }
   
   bool isInferred;
};

class CustomId : public CustomType {
public:
   CustomId(std::string n) { typeValue = TYPE_ID; ofType = nullptr; size = -1; name = n; }

   virtual void printOn(std::ostream &out) const override {
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

   virtual std::string getName() override { return "CustomId"; }

   virtual std::vector<CustomType *> getParams() { return params; }

   virtual void pushToParams(CustomType *newParam) { params.push_back(newParam); }

   virtual void replaceParam(CustomType *newType, int i) { params.at(i) = newType; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

private:
   std::vector<CustomType *> params;
};

class Unknown : public CustomType {
public:
   Unknown() { typeValue = TYPE_UNKNOWN; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      if (size == -1) out << "Unknown"; else out << "None";
      if (SHOW_MEM) out << this;
      if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
   }

   virtual std::string getName() override { return "Unknown"; }

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

#endif