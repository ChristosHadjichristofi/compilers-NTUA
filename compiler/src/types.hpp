#include "ast.hpp"
#include "error.hpp"
#include <string>

enum Types {TYPE_UNIT, TYPE_INT, TYPE_CHAR, TYPE_BOOL, TYPE_FLOAT, TYPE_FUNC, TYPE_REF, TYPE_ARRAY, TYPE_ID };

class Type : public AST {
public:
   virtual void printOn(std::ostream &out) const override {
      out << "Type()";
   }
   virtual bool operator==(const Type &that) const { return false; }

Types typeValue;
Type *ofType;
int size;
std::string name = NULL;
};

class Unit : public Type {
public:
   Unit() { typeValue = TYPE_UNIT; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Unit()";
   }

};

class Integer : public Type {
public:
   Integer() { typeValue = TYPE_INT; ofType = nullptr; size -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Integer()";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_INT){
         return true;
      }
      return false;
  }

};

class Character : public Type {
public:
   Character() { typeValue = TYPE_CHAR; ofType = nullptr; size = -1; }
   
   virtual void printOn(std::ostream &out) const override {
      out << "Character()";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_CHAR){
         return true;
      }
      return false;
   }

};

class Boolean : public Type {
public:   
   Boolean() { typeValue = TYPE_BOOL; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Boolean()";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_BOOL){
         return true;
      }
      return false;
   }

};

class Float : public Type {
public:
   Float() { typeValue = TYPE_FLOAT; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Float()";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_FLOAT){
         return true;
      }
      return false;
   }

};

class Function : public Type {
public:
   Function(Type *it , Type *ot): inputType(it), outputType(ot) { typeValue = TYPE_FUNC; ofType = nullptr; size = -1; }

   virtual void printOn(std::ostream &out) const override {
      out << "Function("; inputType->printOn(out); out << ", "; outputType->printOn(out); out << ")";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_FUNC){
         return true;
      }
      return false;
   }

Type *inputType;
Type *outputType;
};

class Reference : public Type {
public:
   Reference(Type *ct) { 
      typeValue = TYPE_REF; 
      if(ct->typeValue == TYPE_ARRAY){
         yyerror("Input cannot be of type 'Array'");
      }
      ofType = ct;
      size = -1; 
   }

   virtual void printOn(std::ostream &out) const override {
      out << "Reference(ofType:"; ofType->printOn(out); out << ")";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_REF){
         return true;
      }
      return false;
   }

};

class Array : public Type {
public:
   Array(Type *ct, int s) {
      typeValue = TYPE_ARRAY;
      if(ct->typeValue == TYPE_ARRAY){
         yyerror("Input cannot be of type 'Array'");
      }
      ofType = ct;
      size = s;
   }

   virtual void printOn(std::ostream &out) const override {
      out << "Array(ofType"; ofType->printOn(out); out <<", size:" << size <<")";
   }
   
};

class CustomId : public Type {
public:
   CustomId(std::string n) { typeValue = TYPE_ID; ofType = nullptr; size -1; name = n; }

   virtual void printOn(std::ostream &out) const override {
      out << name << "()";
   }

   virtual bool operator==(const Type &inputType) const override {
      if(inputType.typeValue == TYPE_ID && inputType.name == name){
         return true;
      }
      return false;
  }
};


