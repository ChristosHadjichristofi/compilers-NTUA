#include <iomanip>
#include "symbol.hpp"

Color::Modifier redBG(Color::BG_RED);
Color::Modifier yellowBG(Color::BG_YELLOW);
Color::Modifier greenBG(Color::BG_GREEN);
Color::Modifier defBG(Color::BG_DEFAULT);
Color::Modifier defFG(Color::FG_DEFAULT);
Color::Modifier blackFG(Color::FG_BLACK);
Color::Modifier blueFG(Color::FG_BLUE);
Color::Modifier greenFG(Color::FG_GREEN);

/************************************/
/*            SYMBOLENTRY           */
/************************************/

SymbolEntry::SymbolEntry() {}

SymbolEntry::SymbolEntry(CustomType *t): type(t) {}

SymbolEntry::SymbolEntry(std::string str, CustomType *t): id(str), type(t) {}

/************************************/
/*               SCOPE              */
/************************************/

Scope::Scope() {}

// ENTRY_PARAMETER, ENTRY_CONSTRUCTOR, ENTRY_TYPE
bool Scope::lookup(std::string str, int size, EntryTypes entryType) {
    for (int i = size - 1; i > 0; i--) {
        if (locals.find(std::make_pair(str, i)) != locals.end())
        if (entryType == (locals[std::make_pair(str, i)])->entryType && locals[std::make_pair(str,i)]->isVisible) { return true; /* Print Error - duplicate ENTRY_PARAMETER, ENTRY_CONSTRUCTOR, ENTRY_TYPE */ }
    }
    return false;
}

SymbolEntry *Scope::lookup(int size, std::string str, EntryTypes entryType) {
    for (int i = size - 1; i > 0; i--) {
        if (locals.find(std::make_pair(str, i)) != locals.end())
        if (entryType == (locals[std::make_pair(str, i)])->entryType && locals[std::make_pair(str,i)]->isVisible) return locals[std::make_pair(str, i)];
    }
    return nullptr;
}

SymbolEntry *Scope::lookup(std::string str, int size) {
    for (int i = size - 1; i > 0; i--) {
        if (locals.find(std::make_pair(str, i)) == locals.end()) continue;
        if (!locals.find(std::make_pair(str, i))->second->isVisible) continue;
        return locals[std::make_pair(str, i)];
    }
    return nullptr;
}

SymbolEntry *Scope::nonTempLookup(std::string str, int size) {
    for (int i = size - 1; i > 0; i--) {
        if (locals.find(std::make_pair(str, i)) == locals.end()) continue;
        if (locals.find(std::make_pair(str, i))->second->entryType == ENTRY_TEMP) continue;
        return locals[std::make_pair(str, i)];
    }
    return nullptr;
}

void Scope::insert(std::pair<std::string, int> p, CustomType *t, EntryTypes entryType) {
    if(locals.find(p) != locals.end()) { /* later */ }
    else {
        locals[p] = new SymbolEntry(p.first, t);
        locals[p]->entryType = entryType;
        lastEntry = locals[p];
    }
}

void Scope::insert(std::pair<std::string, int> p, SymbolEntry *symbolEntry) {
    if(locals.find(p) != locals.end()) { /* later */ }
    else {
        locals[p] = symbolEntry;
        lastEntry = locals[p];
    }
}

SymbolEntry *Scope::getLastEntry() { return lastEntry; }

/************************************/
/*           PSEUDOSCOPE            */
/************************************/

pseudoScope::pseudoScope() {}

pseudoScope *pseudoScope::getNext() { return scopes.at(currIndex++); }

pseudoScope *pseudoScope::getPrev() { return prevPseudoScope; }

std::pair<int, int> pseudoScope::getTableFormat(int i, std::pair<int, int> currPair, bool recMode) {
    int currMax1 = currPair.first, currMax2 = currPair.second;
    std::pair<int, int> res;
    if (scope != nullptr) {
        if (scope->locals.size() == 31 && i == 1) {}
        else {
            for (auto const& p : scope->locals) {
                int nameLength = p.second->id.length();
                int typeLength = p.second->type->getTypeName().length();
                if (typeLength > currMax2) currMax2 = typeLength;
                if (nameLength > currMax1) currMax1 = nameLength;            
            }
            res = std::make_pair(currMax1, currMax2);
        }
    }
    if (recMode) {
        for (auto s : scopes) {
            std::pair<int, int> tempRes;
            tempRes = s->getTableFormat(i+1, res);
            if (tempRes.first >= res.first && tempRes.second >= res.second) res = tempRes;
            else if (tempRes.first >= res.first && tempRes.second <= res.second) res = std::make_pair(tempRes.first, res.second);
            else if (tempRes.first <= res.first && tempRes.second >= res.second) res = std::make_pair(res.first, tempRes.second);
        }
    }
    return res;
}

void pseudoScope::printPseudoScope(int i, bool recMode) {
    if (scope != nullptr) {
        std::cout << "====================================================== \nSCOPE: " << i << "\n";
        if (scope->locals.size() == 31 && i == 1) {
            std::cout <<"\n*LIBRARY FUNCS (31)*\n\n";
        }
        else
        for (auto const& p : scope->locals) {
            std::cout << "\nSymbol Entry: " << "\n    ID: " << p.second->id;
            std::cout.flush();
            if (SHOW_MEM) std::cout << " [" << p.second << "]";
            std::cout << "\n\n    TYPE: ";
            p.second->type->printOn(std::cout); if (SHOW_MEM) std::cout << "  MEM:  " << p.second->type;
            std::cout << "\n    VISIBLE: " << p.second->isVisible;
            std::cout << "\n    FREEVAR: " << p.second->isFreeVar << std::endl;
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

    if (recMode) for (auto s : scopes) s->printPseudoScope(i+1);
}

void pseudoScope::printPseudoScopeTableFormat(int i, std::pair<int, int> f, bool recMode) {
    if (scope != nullptr) {
        if (scope->locals.size() == 31 && i == 1) {}
        else {
            for (auto const &p : scope->locals) {
                std::cout << std::left << std::setw(10) << std::setfill(' ') << i;
                std::cout << std::left << std::setw(f.first) << std::setfill(' ') << p.second->id;
                std::cout << std::left << std::setw(f.second) << std::setfill(' ') << p.second->type->getTypeName();
                std::cout << std::left << std::setw(8) << std::setfill(' ') << ((p.second->isFreeVar) ? "Yes" : "No") << std::endl;
                if (!p.second->params.empty()) {
                    for (auto par : p.second->params) {
                        std::cout << std::left << std::setw(10) << std::setfill(' ') << i;
                        std::cout << std::left << std::setw(f.first) << std::setfill(' ') << par->id;
                        std::cout << std::left << std::setw(f.second) << std::setfill(' ') << par->type->getTypeName();
                        std::cout << std::left << std::setw(8) << std::setfill(' ') << ((par->isFreeVar) ? "Yes" : "No") << std::endl;
                    }
                }
            }
        }
    }

    if (recMode) for (auto s : scopes) s->printPseudoScopeTableFormat(i + 1, f);
}

SymbolEntry *pseudoScope::lookup(std::string str, int size) {
    for (int i = size - 1; i > 0; i--) {
        if (scope == nullptr) break;
        if (scope->locals.find(std::make_pair(str, i)) == scope->locals.end()) continue;
        if (!scope->locals.find(std::make_pair(str, i))->second->isVisible) continue;
        return scope->locals[std::make_pair(str, i)];
    }
    if (prevPseudoScope != nullptr) return prevPseudoScope->lookup(str, size);
    else return nullptr;
}

SymbolEntry *pseudoScope::lookupTypes(std::string str, int size) {
    for (int i = size - 1; i > 0; i--) {
        if (scope == nullptr) break;
        if (scope->locals.find(std::make_pair(str, i)) == scope->locals.end()) continue;
        if (!scope->locals.find(std::make_pair(str, i))->second->isVisible) continue;
        /* check if the se that will be returned has typeValue = TYPE_CUSTOM && entryType = ENTRY_TYPE */
        /* every other case continue */
        if (scope->locals.find(std::make_pair(str, i))->second->type->typeValue != TYPE_CUSTOM) continue;
        if (scope->locals.find(std::make_pair(str, i))->second->entryType != ENTRY_TYPE) continue;
        return scope->locals[std::make_pair(str, i)];
    }
    if (prevPseudoScope != nullptr) return prevPseudoScope->lookupTypes(str, size);
    else return nullptr;
}

void pseudoScope::initCurrIndexes() {
    this->currIndex = 0;
    for (auto s : scopes) if (s != nullptr) s->initCurrIndexes();
}

/************************************/
/*       PSEUDOSYMBOLTABLE          */
/************************************/

pseudoScope *currPseudoScope = nullptr;

PseudoSymbolTable::PseudoSymbolTable() { 
    pScope.push_back(new pseudoScope()); 
    currPseudoScope = pScope.front(); 
}

void PseudoSymbolTable::printST(bool tableFormat) {
    int i = 0;
    if (!tableFormat) {
        std::cout << "\n\n $$$ PRINTING COMPLETE SYMBOL TABLE $$$ \n";
        for (auto currPScope : pScope) currPScope->printPseudoScope(i);
    }
    else {
        std::pair<int, int> format;
        for (auto currPScope : pScope) format = currPScope->getTableFormat(i, std::make_pair(0, 0));
        format.first += 8;
        format.second += 8;

        std::cout << blueFG << std::left << std::setw(10) << std::setfill(' ') << "Scope";
        std::cout << std::left << std::setw(format.first) << std::setfill(' ') << "Name";
        std::cout << std::left << std::setw(format.second) << std::setfill(' ') << "Type";
        std::cout << std::left << std::setw(8) << std::setfill(' ') << "isFree" << defFG << std::endl;
        for (auto currPScope : pScope) currPScope->printPseudoScopeTableFormat(i, format);
    }
    std::cout << "\n\n"; std::cout.flush();
}

void PseudoSymbolTable::incrSize() { size++; }

int PseudoSymbolTable::getSize() { return size; }

void PseudoSymbolTable::incrSize(int s) { size += s; }

void PseudoSymbolTable::initSize() { size = 32; }

/************************************/
/*            SYMBOLTABLE           */
/************************************/

void SymbolTable::printST() {
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

void SymbolTable::openScope() {
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

void SymbolTable::closeScope() {
    scopes.pop_back();
    currPseudoScope = currPseudoScope->prevPseudoScope;
}

bool SymbolTable::lookup(std::string str, EntryTypes entryType) {
    bool found = false;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
        found = (*i)->lookup(str, size, entryType);
        if (found) return found;
    }
    return found;
}

SymbolEntry *SymbolTable::lookup(EntryTypes entryType, std::string str) {
    SymbolEntry *stEntry;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
        stEntry = (*i)->lookup(size, str, entryType);
        if(stEntry) return stEntry;
    }
    return nullptr;
}

SymbolEntry *SymbolTable::lookup(std::string str, bool sameScope) {

    SymbolEntry *entry = nullptr;
    if(!sameScope)
        for(auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
        entry = (*i)->lookup(str, size);
        if(entry != nullptr) return entry;
        }
    else {
        entry = (*scopes.rbegin())->lookup(str, size);
        if(entry != nullptr) return entry;
    }
    // error 404
    return nullptr;
}

SymbolEntry *SymbolTable::nonTempLookup(std::string str) {

    SymbolEntry *entry = nullptr;
    for(auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
        entry = (*i)->nonTempLookup(str, size);
        if(entry != nullptr) return entry;
    }
    // error 404
    return nullptr;
}

SymbolEntry *SymbolTable::getLastEntry() {
    if (scopes.back()->locals.size() != 0) return scopes.back()->getLastEntry();
    else return scopes.rbegin()[1]->getLastEntry();
}

void SymbolTable::insert(std::string str, CustomType *t, EntryTypes entryType) {
    scopes.back()->insert(std::make_pair(str, size++), t, entryType);
}

void SymbolTable::insert(std::string str, SymbolEntry *symbolEntry) {
    scopes.back()->insert(std::make_pair(str, size++), symbolEntry);
}

int SymbolTable::getSize() { return size; }