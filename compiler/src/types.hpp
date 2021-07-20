#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include "AST.hpp"
#include "lexer.hpp"
#include <string>

enum Types { TYPE_UNIT, TYPE_INT, TYPE_CHAR, TYPE_BOOL, TYPE_FLOAT, TYPE_FUNC, TYPE_REF, TYPE_ARRAY, TYPE_ID, TYPE_STR, TYPE_UNKNOWN };

class CustomType : public AST {
public:
   virtual void printOn(std::ostream &out) const override {
      out << "CustomType()";
   }
   
   virtual bool operator==(const CustomType &that) const { return false; }

Types typeValue;
CustomType *ofType;
int size;
std::string name;
};

class Unit : public CustomType {
public:
   Unit() { typeValue = TYPE_UNIT; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Unit()";
   }

};

class Integer : public CustomType {
public:
   Integer() { typeValue = TYPE_INT; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Integer()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_INT){
         return true;
      }
      return false;
  }

};

class String : public CustomType {
public:
   String() { typeValue = TYPE_STR; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "String()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_STR){
         return true;
      }
      return false;
   }
};

class Character : public CustomType {
public:
   Character() { typeValue = TYPE_CHAR; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Character()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_CHAR){
         return true;
      }
      return false;
   }

};

class Boolean : public CustomType {
public:   
   Boolean() { typeValue = TYPE_BOOL; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Boolean()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_BOOL){
         return true;
      }
      return false;
   }

};

class Float : public CustomType {
public:
   Float() { typeValue = TYPE_FLOAT; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Float()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_FLOAT){
         return true;
      }
      return false;
   }

};

class Function : public CustomType {
public:
   Function(/*CustomType *it ,*/ CustomType *ot) { outputType = ot; typeValue = TYPE_FUNC; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Function("; 
      if (!params.empty()) {
         out << "{ ";
         for (auto i : params) { i->printOn(out); out << " "; } 
         out << "}, "; 
      } 
      outputType->printOn(out); 
      out << ")";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_FUNC){
         return true;
      }
      return false;
   }

std::vector<CustomType *> params;
CustomType *outputType;
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
      out << "Reference(ofType:"; ofType->printOn(out); out << ")";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_REF){
         return true;
      }
      return false;
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
   }

   virtual void printOn(std::ostream &out) const override {
      out << "Array(ofType"; ofType->printOn(out); out <<", size:" << size <<")";
   }
   
};

class CustomId : public CustomType {
public:
   CustomId(std::string n) { typeValue = TYPE_ID; ofType = nullptr; size = -1; name = n; }

   virtual void printOn(std::ostream &out) const override {
      out << name << "()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_ID && inputType.name == name){
         return true;
      }
      return false;
  }

   virtual std::vector<CustomType *> getParams() { return params; }

private:
   std::vector<CustomType *> params;
};

class Unknown : public CustomType {
public:
   Unknown() { typeValue = TYPE_UNKNOWN; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Unknown()";
   }

   virtual bool operator==(const CustomType &inputType) const override {
      if(inputType.typeValue == TYPE_UNKNOWN){
         return true;
      }
      return false;
  }

};

#endif