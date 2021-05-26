#include <iostream>
#include <vector>
#include <map>
#include "types.hpp"

struct SymbolEntry {
   CustomType *type;
   SymbolEntry() {}
   SymbolEntry(CustomType *t): type(t) {}
};

class Scope {
public:
   Scope() {}

   SymbolEntry *lookup(std::string str){
      if(locals.find(str) == locals.end()) return nullptr;
      return &(locals[str]);
   }

   void update(std::string str, CustomType *t){
      locals[str] = SymbolEntry(t);
   }

   void insert(std::string str, CustomType *t) {
      if(locals.find(str) != locals.end()) 
         update(str, t);
      else 
         locals[str] = SymbolEntry(t);
   }

 

private:
std::map<std::string, SymbolEntry> locals;
int size;
};

class SymbolTable {
public:

   void openScope(){
      scopes.push_back(Scope());
   }

   void closeScope(){
      scopes.pop_back();
   }

   SymbolEntry *lookup(std::string str){
      SymbolEntry *entry;
      for(auto i = scopes.rbegin(); i != scopes.rend(); ++i){
         entry = i->lookup(str);
         if(entry) return entry;
      }
      // error 404
   }

   void insert(std::string str, CustomType *t) {
      scopes.back().insert(str, t);
   }

private:
std::vector<Scope> scopes;
};

extern SymbolTable st;