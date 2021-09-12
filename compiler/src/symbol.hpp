#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include "types.hpp"

enum EntryTypes { ENTRY_CONSTANT, ENTRY_FUNCTION, ENTRY_PARAMETER, ENTRY_VARIABLE, ENTRY_TYPE, ENTRY_CONSTRUCTOR, ENTRY_IDENTIFIER, ENTRY_TEMP };

struct SymbolEntry {
   std::string id;
   CustomType *type;
   /* 
   Is used to hold:
    - Parameters of a function
    - Constructors of a user defined type
    - User defined type in a Constructor (single argument)   
   */
   std::vector<SymbolEntry *> params;
   EntryTypes entryType;
   int counter;
   bool isVisible = true;
   bool sameMemAsOutput = false;
   llvm::Value *Value;
   llvm::Type *LLVMType;
   llvm::Function *Function;

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
            if (entryType == (locals[std::make_pair(str, i)])->entryType && locals[std::make_pair(str,i)]->isVisible) { return true; /* Print Error - duplicate ENTRY_PARAMETER, ENTRY_CONSTRUCTOR, ENTRY_TYPE */ }
      }
      return false;
   }

   SymbolEntry *lookup(int size, std::string str, EntryTypes entryType) {
      for (int i = size - 1; i > 0; i--) {
         if (locals.find(std::make_pair(str, i)) != locals.end())
            if (entryType == (locals[std::make_pair(str, i)])->entryType && locals[std::make_pair(str,i)]->isVisible) return locals[std::make_pair(str, i)];
      }
      return nullptr;
   }

   SymbolEntry *lookup(std::string str, int size) {
      for (int i = size - 1; i > 0; i--) {
         if (locals.find(std::make_pair(str, i)) == locals.end()) continue;
         if (!locals.find(std::make_pair(str, i))->second->isVisible) continue;
         return locals[std::make_pair(str, i)];
      }
      return nullptr;
   }

   SymbolEntry *nonTempLookup(std::string str, int size) {
      for (int i = size - 1; i > 0; i--) {
         if (locals.find(std::make_pair(str, i)) == locals.end()) continue;
         if (locals.find(std::make_pair(str, i))->second->entryType == ENTRY_TEMP) continue;
         return locals[std::make_pair(str, i)];
      }
      return nullptr;
   }

   void insert(std::pair<std::string, int> p, CustomType *t, EntryTypes entryType) {
      if(locals.find(p) != locals.end()) { /* later */ }
      else {
         locals[p] = new SymbolEntry(p.first, t);
         locals[p]->entryType = entryType;
         lastEntry = locals[p];
      }
   }

   void insert(std::pair<std::string, int> p, SymbolEntry *symbolEntry) {
      if(locals.find(p) != locals.end()) { /* later */ }
      else {
         locals[p] = symbolEntry;
         lastEntry = locals[p];
      }
   }
   
   SymbolEntry *getLastEntry() { return lastEntry; }

   /* change to unordered map */
   std::map<std::pair<std::string, int>, SymbolEntry*> locals;

private:
   int size;
   SymbolEntry *lastEntry;
};

static bool flag = false;

/* PseudoScope */
class pseudoScope {
public:
   pseudoScope() {}
   Scope *scope;
   std::vector<pseudoScope *> scopes;
   pseudoScope *prevPseudoScope;
   int currIndex = 0;

   pseudoScope *getNext() { return scopes.at(currIndex++); }

   pseudoScope *getPrev() { return prevPseudoScope; }

   void printPseudoScope(int i, bool recMode = true) {
      if (scope != nullptr) {
         if (flag) {
            std::cout << "====================================================== \nSCOPE: " << i << "\n";
            for (auto const& p : scope->locals) {
               std::cout << "\nSymbol Entry: " << "\n    ID: " << p.second->id;
               std::cout.flush();
               if (SHOW_MEM) std::cout << " [" << p.second << "]";
               std::cout << "\n\n    TYPE: ";
               p.second->type->printOn(std::cout); if (SHOW_MEM) std::cout << "  MEM:  " << p.second->type;
               if (!p.second->params.empty()) {
                  std::cout << "\n\n    PARAMS:\n";
                  for (auto i : p.second->params) {
                     std::cout << "\t\tName: " << i->id << ", Type: ";
                     i->type->printOn(std::cout); std::cout << std::endl;
                     if (SHOW_MEM) std::cout << " MEM OF TYPE: " << i->type << "\n";
                  }
               }
               std::cout << '\n';
            }
         }
         if(!scope->locals.empty()) flag = true;
      }

      if (recMode) for (auto s : scopes) s->printPseudoScope(i++);
   }

   SymbolEntry *lookup(std::string str, int size) {
      for (int i = size - 1; i > 0; i--) {
         if (scope == nullptr) break;
         if (scope->locals.find(std::make_pair(str, i)) == scope->locals.end()) continue;
         if (!scope->locals.find(std::make_pair(str, i))->second->isVisible) continue;
         return scope->locals[std::make_pair(str, i)];
      }
      if (prevPseudoScope != nullptr) return prevPseudoScope->lookup(str, size);
      else return nullptr;
   }

};

static pseudoScope *currPseudoScope;

class PseudoSymbolTable {
public:
   PseudoSymbolTable() { pScope.push_back(new pseudoScope()); currPseudoScope = pScope.front(); }
   std::vector<pseudoScope *> pScope;

   void printST() {
      int i = 0;
      std::cout << "\n\n $$$ PRINTING COMPLETE SYMBOL TABLE $$$ \n";
      for (auto currPScope : pScope) currPScope->printPseudoScope(i);
      
      std::cout << "\n\n"; std::cout.flush();
   }
};

class SymbolTable {
public:

   void printST() {
      int i = 0;
      std::cout << "\n\n $$$ PRINTING SYMBOL TABLE $$$ \n";
      for (auto scope : scopes) {
         if (i == 0) { i++; continue; }
         std::cout << " ====================================================== \nSCOPE: " << i++ << "\n";
         for (auto const& p : scope->locals) {
            std::cout << "\nSymbol Entry: " << "\n    ID: " << p.second->id;
            if (SHOW_MEM) std::cout << " [" << p.second << "]";
            std::cout << "\n\n    TYPE: ";
            p.second->type->printOn(std::cout); if (SHOW_MEM) std::cout << "  MEM:  " << p.second->type;
            if (!p.second->params.empty()) {
               std::cout << "\n\n    PARAMS:\n";
               for (auto i : p.second->params) {
                  std::cout << "\t\tName: " << i->id << ", Type: ";
                  i->type->printOn(std::cout); std::cout << std::endl;
                  if (SHOW_MEM) std::cout << " MEM OF TYPE: " << i->type << "\n";
               }
            }
            std::cout << '\n';
         }
      }
      std::cout << "\n\n"; std::cout.flush();
   }

   void openScope() {
      // std::cout << "Opening Scope ... " << std::endl;
      scopes.push_back(new Scope());
      /* Push back new pseudoscope in scopes */
      currPseudoScope->scopes.push_back(new pseudoScope());
      /* set next's scope to point to actual scope */
      currPseudoScope->scopes.back()->scope = scopes.back();
      /* set next's previous to this */
      currPseudoScope->scopes.back()->prevPseudoScope = currPseudoScope;
      /* set this to next */
      currPseudoScope = currPseudoScope->scopes.back();
   }

   void closeScope() {
      scopes.pop_back();
      currPseudoScope = currPseudoScope->prevPseudoScope;
   }

   bool lookup(std::string str, EntryTypes entryType) {
      bool found = false;
      for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         found = (*i)->lookup(str, size, entryType);
         if (found) return found;
      }
      return found;
   }

   SymbolEntry *lookup(EntryTypes entryType, std::string str) {
      SymbolEntry *stEntry;
      for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         stEntry = (*i)->lookup(size, str, entryType);
         if(stEntry) return stEntry;
      }
      return nullptr;
   }

   SymbolEntry *lookup(std::string str){

      SymbolEntry *entry = nullptr;
      for(auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         entry = (*i)->lookup(str, size);
         if(entry != nullptr) return entry;
      }
      // error 404
      return nullptr;
   }

   SymbolEntry *nonTempLookup(std::string str){

      SymbolEntry *entry = nullptr;
      for(auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
         entry = (*i)->nonTempLookup(str, size);
         if(entry != nullptr) return entry;
      }
      // error 404
      return nullptr;
   }

   SymbolEntry *getLastEntry() {
      if (scopes.back()->locals.size() != 0) return scopes.back()->getLastEntry();
      else return scopes.rbegin()[1]->getLastEntry();
   }

   void insert(std::string str, CustomType *t, EntryTypes entryType) {
      scopes.back()->insert(std::make_pair(str, size++), t, entryType);
   }

   void insert(std::string str, SymbolEntry *symbolEntry) {
      scopes.back()->insert(std::make_pair(str, size++), symbolEntry);
   }

   int getSize() { return size; }

private:
   std::vector<Scope *> scopes;
   int size = 1;
};

extern SymbolTable st;
extern PseudoSymbolTable pseudoST;