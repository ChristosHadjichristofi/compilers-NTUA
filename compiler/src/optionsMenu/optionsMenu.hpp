#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <bits/stdc++.h>
#include "../ast/ast.hpp"

class Option {

public:
    Option();
    Option(std::string _flag, std::string _description, bool _used);
    std::string getFlag();
    std::string getDescription();
    bool getUsed();
    void setUsed();

protected:
    std::string flag;
    std::string description;
    bool used;

};

class OptionsMenu {

public:
    OptionsMenu();
    void appendOption(Option *o);
    void setStmtList(AST *_root);
    AST *getStmtList();
    void execute();
    std::vector<Option *> getOptions();
    void init();
    void parse(int argc, char **argv);
    void print();
    std::string getFile() { return file; }
    void setFile(std::string f) { file = f; }

protected:
    std::vector<Option *> options;
    AST *root;
    std::string file;

};

extern OptionsMenu *optionsMenu;