#include <iostream>
#include <vector>
#include <string>
#include "../ast/ast.hpp"

class Option {

public:
    Option();
    Option(std::string _flag, std::string _description, bool _used);
    void execute();
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
    std::vector<Option *> getOptions();
    void init();
    void parse(int argc, char **argv);
    void print();

protected:
    std::vector<Option *> options;
    AST *root;

};

extern OptionsMenu *optionsMenu;