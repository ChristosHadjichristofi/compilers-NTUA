#ifndef __SYMBOL_HPP__
#define __SYMBOL_HPP__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include "../types/types.hpp"
#include "../ast/colormod.hpp"

extern Color::Modifier redBG;
extern Color::Modifier greenBG;
extern Color::Modifier yellowBG;
extern Color::Modifier defBG;
extern Color::Modifier defFG;
extern Color::Modifier blackFG;
extern Color::Modifier blueFG;
extern Color::Modifier greenFG;

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
   bool isFreeVar = false;
   llvm::Value *Value;
   llvm::Type *LLVMType;
   llvm::Type *env = nullptr;
   llvm::Value *functionEnvPtr = nullptr;
   llvm::Value *GlobalValue = nullptr;

   SymbolEntry();
   SymbolEntry(CustomType *t);
   SymbolEntry(std::string str, CustomType *t);
};

class Scope {
public:

   Scope();
   // ENTRY_PARAMETER, ENTRY_CONSTRUCTOR, ENTRY_TYPE
   bool lookup(std::string str, int size, EntryTypes entryType);
   SymbolEntry *lookup(int size, std::string str, EntryTypes entryType);
   SymbolEntry *lookup(std::string str, int size);
   SymbolEntry *nonTempLookup(std::string str, int size);
   void insert(std::pair<std::string, int> p, CustomType *t, EntryTypes entryType);
   void insert(std::pair<std::string, int> p, SymbolEntry *symbolEntry);
   SymbolEntry *getLastEntry();

   /* change to unordered map */
   std::map<std::pair<std::string, int>, SymbolEntry*> locals;

private:
   int size;
   SymbolEntry *lastEntry;
};

extern bool flag;

/* PseudoScope */
class pseudoScope {
public:
   pseudoScope();
   pseudoScope *getNext();
   pseudoScope *getPrev();
   std::pair<int, int> getTableFormat(int i, std::pair<int, int> currPair, bool recMode = true);
   void printPseudoScope(int i, bool recMode = true);
   void printPseudoScopeTableFormat(int i, std::pair<int, int> f, bool recMode = true);
   SymbolEntry *lookup(std::string str, int size);
   SymbolEntry *lookupTypes(std::string str, int size);
   void initCurrIndexes();

   Scope *scope;
   std::vector<pseudoScope *> scopes;
   pseudoScope *prevPseudoScope;
   int currIndex = 0;

};

extern pseudoScope *currPseudoScope;

class PseudoSymbolTable {
public:
   PseudoSymbolTable();
   void printST(bool tableFormat = true);
   void incrSize();
   int getSize();
   void incrSize(int s);
   void initSize();

   std::vector<pseudoScope *> pScope;

private:
   /* starts from 32 due to library functions */
   int size = 32;
};

class SymbolTable {
public:
   void printST();
   void openScope();
   void closeScope();
   bool lookup(std::string str, EntryTypes entryType);
   SymbolEntry *lookup(EntryTypes entryType, std::string str);
   SymbolEntry *lookup(std::string str, bool sameScope = false);
   SymbolEntry *nonTempLookup(std::string str);
   SymbolEntry *getLastEntry();
   void insert(std::string str, CustomType *t, EntryTypes entryType);
   void insert(std::string str, SymbolEntry *symbolEntry);
   int getSize();


private:
   std::vector<Scope *> scopes;
   int size = 1;
};

extern SymbolTable st;
extern PseudoSymbolTable pseudoST;

#endif