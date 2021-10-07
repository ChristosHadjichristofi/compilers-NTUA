#include "optionsMenu.hpp"

/************************************/
/*              OPTION              */
/************************************/

Option::Option() {}

Option::Option(std::string _flag, std::string _description, bool _used) {
    flag = _flag;
    description = _description;
    used = _used;
}

void Option::execute() {
    if (getFlag() == "-p") optionsMenu->getStmtList()->printOn(std::cout);
    else if (getFlag() == "-O") optionsMenu->getStmtList()->printOn(std::cout);
    else if (getFlag() == "-i") optionsMenu->getStmtList()->printOn(std::cout);
    else optionsMenu->getStmtList()->printOn(std::cout);
}

std::string Option::getFlag() { return flag; }

std::string Option::getDescription() { return description; }

void Option::setUsed() { used = true; }

bool Option::getUsed() { return used; }

/************************************/
/*            OPTIONSMENU           */
/************************************/

OptionsMenu::OptionsMenu() {}

void OptionsMenu::appendOption(Option *o) {
    options.push_back(o);
}

void OptionsMenu::setStmtList(AST *_root) { root = _root; }

AST *OptionsMenu::getStmtList() { return root; }

std::vector<Option *> OptionsMenu::getOptions() { return options; }

void OptionsMenu::init() {
    
    /* create the options */
    Option *printAST = new Option("-p", "Print the AST.", false);
    Option *optimization = new Option("-O", "Optimization.", false);
    Option *intermediateCode = new Option("-i", "Prints Intermediate Code.", false);
    Option *assemblyCode = new Option("-f", "Prints Assembly Code.", false);
    Option *help = new Option("-h", "Usage: ./llama *.lla [flags]", false);

    /* append them to the options menu */
    optionsMenu->appendOption(printAST);
    optionsMenu->appendOption(optimization);
    optionsMenu->appendOption(intermediateCode);
    optionsMenu->appendOption(assemblyCode);
    optionsMenu->appendOption(help);

}

void OptionsMenu::parse(int argc, char **argv) {

    if (argc == 1) {
        std::cout << "Llama can not run without any args\n";
        optionsMenu->print();
        exit(1);
    }

    if (std::freopen(argv[1], "r", stdin) == nullptr) {
        std::cout << "File could not be opened\n";
        exit(1);
    }

    for (int i = 2; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }

}

void OptionsMenu::print() {
    
    std::cout << "Options:\n"; 
    for (Option *o : options) {
        std::cout << "Flag: " << o->getFlag() << " | " << "Description: " << o->getDescription() << std::endl;
    }
}

OptionsMenu *optionsMenu = new OptionsMenu();