#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include "types.hpp"

enum EntryTypes { ENTRY_CONSTANT, ENTRY_FUNCTION, ENTRY_PARAMETER, ENTRY_VARIABLE, ENTRY_TYPE, ENTRY_CONSTRUCTOR, ENTRY_IDENTIFIER };

struct SymbolEntry {
   std::string id;
   CustomType *type;
   std::vector<SymbolEntry *> params;
   EntryTypes entryType;
   int counter;
   // Value
   // Function
   SymbolEntry() {}
   SymbolEntry(CustomType *t): type(t) {}
   SymbolEntry(std::string str, CustomType *t): id(str), type(t) {}
};

class Scope {
public:

   Scope() {}

   // ENTRY_PARAMETER, ENTRY_CONSTRUCTOR, ENTRY_TYPE
   bool lookup(std::string str, int size, EntryTypes entryType) {
      for (int i = size - 1; i > 0; i--) {
         if (locals.find(std::make_pair(str, i)) != locals.end())
            if (entryType == (&locals[std::make_pair(str, i)])->entryType) { return true; /* Print Error - duplicate ENTRY_PARAMETER, ENTRY_CONSTRUCTOR, ENTRY_TYPE */ }
      }
      return false;
   }

   SymbolEntry *lookup(int size, std::string str, EntryTypes entryType) {
      for (int i = size - 1; i > 0; i--) {
         if (locals.find(std::make_pair(str, i)) != locals.end())
            if (entryType == (&locals[std::make_pair(str, i)])->entryType) return &locals[std::make_pair(str, i)];
      }
      return nullptr;
   }

   SymbolEntry *lookup(std::string str, int size) {
      for (int i = size - 1; i > 0; i--) {
         if(locals.find(std::make_pair(str, i)) == locals.end()) continue;
         return &(locals[std::make_pair(str, i)]);
      }
      return nullptr;
   }

   void insert(std::pair<std::string, int> p, CustomType *t, EntryTypes entryType) {
      if(locals.find(p) != locals.end()) { /* later */ }
      else {
         locals[p] = SymbolEntry(p.first, t);
         locals[p].entryType = entryType;
         lastEntry = &locals[p];
      }
   }
   
   SymbolEntry *getLastEntry() { return lastEntry; }

   std::map<std::pair<std::string, int>, SymbolEntry> locals;
private:
/* change to unordered map */
int size;
SymbolEntry *lastEntry;
};

class SymbolTable {
public:

   void openScope(){
      std::cout << "Opening Scope ... " << std::endl;
      scopes.push_back(Scope());
   }
   void closeScope() {
      std::cout << "Closing Scope ... " << std::endl;
      for (auto const& p : scopes.back().locals) {
         std::cout << "Symbol Entry: " << "\n    ID: " << p.second.id << "\n    TYPE: ";
         p.second.type->printOn(std::cout);
         if (!p.second.params.empty()) {
            std::cout << "\n    PARAMS: ";
            for (auto i : p.second.params) {
               i->type->printOn(std::cout);
               std::cout << " ";
            }
         }
         std::cout << '\n';
      }
      scopes.pop_back();
   }
   // void closeScope(){
   //    scopes.pop_back();
   // }

   bool lookup(std::string str, EntryTypes entryType) {
      bool found = false;
      for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         found = i->lookup(str, size, entryType);
         if (found) return found;
      }
      return found;
   }

   SymbolEntry *lookup(EntryTypes entryType, std::string str) {
      SymbolEntry *stEntry;
      for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         stEntry = i->lookup(size, str, entryType);
         if (!stEntry) return stEntry;
      }
      return nullptr;
   }

   SymbolEntry *lookup(std::string str){
      SymbolEntry *entry;
      for(auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         entry = i->lookup(str, size);
         if(entry) {
            std::cout << "Returning Symbol Entry: " << "\n    ID: " << entry->id << "\n    TYPE: ";
            entry->type->printOn(std::cout);
            if (!entry->params.empty()) {
               std::cout << "\n    PARAMS: ";
               for (auto i : entry->params) {
                  i->type->printOn(std::cout);
                  std::cout << " ";
               }
            }
            std::cout << '\n';
         
            return entry;
         }
      }
      // error 404
      return nullptr;
   }

   SymbolEntry *getLastEntry() {
      if (scopes.back().locals.size() != 0) return scopes.back().getLastEntry();
      else return scopes.rbegin()[1].getLastEntry();
   }

   void insert(std::string str, CustomType *t, EntryTypes entryType) {
      scopes.back().insert(std::make_pair(str, size++), t, entryType);
   }

private:
std::vector<Scope> scopes;
int size = 1;
};

extern SymbolTable st;