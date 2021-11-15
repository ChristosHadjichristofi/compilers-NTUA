#include "optionsMenu.hpp"

bool SHOW_LINE_MACRO = false;

/******************************************/
/* Function to check the format of a file */
/******************************************/

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) 
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    else 
        return false;
}

/************************************/
/*              OPTION              */
/************************************/

Option::Option() {}

Option::Option(std::string _flag, std::string _description, bool _used) {
    flag = _flag;
    description = _description;
    used = _used;
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
    Option *showLineMacro = new Option("-l", "Show line Macro - Debugging Purposes.", false);
    Option *optimization = new Option("-O", "Optimization Flag.", false);
    Option *printAST = new Option("-ast", "Prints the AST.", false);
    Option *printFV = new Option("-fv", "Prints the FreeVariables each Function has.", false);
    Option *intermediateCode = new Option("-i", "Prints Intermediate Code.", false);
    Option *assemblyCode = new Option("-f", "Prints Assembly Code.", false);
    Option *printPseudoST = new Option("-st", "Prints complete SymbolTable (may have duplicates, because of scopes).", false);
    Option *help = new Option("-h", "\033[43m\033[30mUsage:\033[49m\033[39m ./llama *.lla [flags]", false);

    /* append them to the options menu */
    optionsMenu->appendOption(showLineMacro);
    optionsMenu->appendOption(optimization);
    optionsMenu->appendOption(printAST);
    optionsMenu->appendOption(printFV);
    optionsMenu->appendOption(printPseudoST);
    optionsMenu->appendOption(intermediateCode);
    optionsMenu->appendOption(assemblyCode);
    optionsMenu->appendOption(help);

}

void OptionsMenu::parse(int argc, char **argv) {
    
    if (argc == 1) {
        std::cout << redBG << blackFG << "Error:" << defBG << defFG << " Llama can not run without any args\n";
        optionsMenu->print();
        exit(1);
    }

    // check if is lla format
    if (!hasEnding(argv[1], ".lla")) {
        std::cout << redBG << blackFG << "Error:" << defBG << defFG << " Accepted format is .lla\n";
        exit(1);
    }

    // check if file can be opened
    if (std::freopen(argv[1], "r", stdin) == nullptr) {
        std::cout << redBG << blackFG << "Error:" << defBG << defFG << " File could not be opened\n";
        exit(1);
    }

    // add filename to optionsMenu
    optionsMenu->setFile(argv[1]);

    // iterate through args
    for (int i = 2; i < argc; i++) {
        
        // this var will be used to check if given flag is valid
        bool found = false;

        // all options should start with -, if not error
        if (argv[i][0] != '-') {
            std::cout << redBG << blackFG << "Error:" << defBG << defFG << " '" << argv[i] << "' This is not an option.\n";
            optionsMenu->print();
            exit(1);
        }
        else {
            // iterate through options in option menu
            for (Option *o : optionsMenu->getOptions()) {
                // if argv[i] is found in options menu set it to used
                if (o->getFlag() == argv[i]) {
                    o->setUsed();
                    found = true;
                    break;
                }
            }

            // in case that it is not found, then no valid option
            if (!found) {
                std::cout << redBG << blackFG << "Error:" << defBG << defFG << " " << argv[i] << " is not a valid option.\n\n";
                optionsMenu->print();
                exit(1);
            }
        }
    }
}

void OptionsMenu::execute() {

    // get file and set it to .ll and .asm
    std::string file = optionsMenu->getFile();
    std::string fileLL = file.substr(0, file.length() - 3) + "ll";
    std::string fileAsm = file.substr(0, file.length() - 3) + "asm";
    std::string fileOut = file.substr(0, file.length() - 4);

    // help command
    if (optionsMenu->getOptions().at(7)->getUsed()) {
        optionsMenu->print();
        exit(0);
    }

    // enable line macro
    if (optionsMenu->getOptions().at(0)->getUsed()) SHOW_LINE_MACRO = true;

    // print AST
    if (optionsMenu->getOptions().at(2)->getUsed()) {
        optionsMenu->getStmtList()->printOn(std::cout);
        std::cout << std::endl;
    }

    // sem
    optionsMenu->getStmtList()->sem();

    // print FreeVars
    if (optionsMenu->getOptions().at(3)->getUsed()) {
        printFreeVars();
        std::cout << std::endl;
    }

    // print Pseudo ST
    if (optionsMenu->getOptions().at(4)->getUsed()) {
        pseudoST.printST();
        exit(0);
    }

    if (semError) exit(1);
    optionsMenu->getStmtList()->preCompile();
    
    optionsMenu->getStmtList()->llvm_compile_and_dump(fileLL, optionsMenu->getOptions().at(1)->getUsed());

    // compile ir to asm
    std::string command = "clang " + fileLL + " -o " + fileAsm + " -S";
    if (std::system(command.c_str()) == -1) {
        std::cout << redBG << blackFG << "Error:" << defBG << defFG << " while compiling IR to Assembly";
        exit(1);
    }

    // compile asm to exe
    command = "clang -o " + fileOut + " " + fileLL + " library/lib.a -lm";
    if (std::system(command.c_str()) == -1) {
        std::cout << redBG << blackFG << "Error:" << defBG << defFG << " while compiling Assembly to Executable";
        exit(1);
    }
    
    // print IR code
    if (optionsMenu->getOptions().at(5)->getUsed()) {
        std::ifstream f(fileLL);
        if (f.is_open()) std::cout << f.rdbuf();
    }

    // print Assembly code
    if (optionsMenu->getOptions().at(6)->getUsed()) {
        std::ifstream f(fileAsm);
        if (f.is_open()) std::cout << f.rdbuf();
    }
}

void OptionsMenu::print() {
    std::cout << greenFG << std::left << std::setw(14) << std::setfill(' ') << "Flag Options";
    std::cout << greenFG << std::left << std::setw(100) << std::setfill(' ') << "Description" << std::endl;
    for (Option *o : options) {
        std::cout << greenFG << std::left << std::setw(14) << std::setfill(' ') << o->getFlag() << defFG;
        std::cout << std::left << std::setw(100) << std::setfill(' ') << o->getDescription() << std::endl;
    }
}

OptionsMenu *optionsMenu = new OptionsMenu();