#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include "AST.hpp"
#include "lexer.hpp"
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

   virtual llvm::Value* compile() const override {
      return 0;
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

   virtual llvm::Value* compile() const override {
      return 0;
   }

};

#endif