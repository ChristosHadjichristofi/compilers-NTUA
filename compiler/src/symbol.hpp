#include <iostream>
#include <vector>
#include <map>
#include "types.hpp"

/* 
Symbol Entry containing:
   - id name
   - id type 
*/
struct SymbolEntry {
   std::string id;
   CustomType *type;
   // Value
   // Function
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
      else {
         locals[str] = SymbolEntry(t);
         lastEntry = &locals[str];
      }
   }
   
   SymbolEntry *getLastEntry() { return lastEntry; }

private:
std::map<std::string, SymbolEntry> locals;
int size;
SymbolEntry *lastEntry;
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

   SymbolEntry *getLastEntry() {
      if (!scopes.empty()) return scopes.back().getLastEntry();
      else return scopes.rbegin()[1].getLastEntry();
   }

   void insert(std::string str, CustomType *t) {
      scopes.back().insert(str, t);
   }

private:
std::vector<Scope> scopes;
};

extern SymbolTable st;