#include "optionsMenu.hpp"

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
    Option *optimization = new Option("-O", "Optimization Flag.", false);
    Option *printAST = new Option("-p", "Prints the AST.", false);
    Option *intermediateCode = new Option("-i", "Prints Intermediate Code.", false);
    Option *assemblyCode = new Option("-f", "Prints Assembly Code.", false);
    Option *help = new Option("-h", "Usage: ./llama *.lla [flags]", false);

    /* append them to the options menu */
    optionsMenu->appendOption(optimization);
    optionsMenu->appendOption(printAST);
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

    // check if is lla format
    if (!hasEnding(argv[1], ".lla")) {
        std::cout << "Accepted format is .lla\n";
        exit(1);
    }

    // check if file can be opened
    if (std::freopen(argv[1], "r", stdin) == nullptr) {
        std::cout << "File could not be opened\n";
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
            std::cout << "This is not an option.\n";
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
                std::cout << argv[i] << " is not a valid option.\n";
                optionsMenu->print();
                exit(1);
            }
        }
    }
}

void OptionsMenu::execute() {

    bool print = false;

    // get file and set it to .ll and .asm
    std::string file = optionsMenu->getFile();
    std::string fileLL = file.substr(0, file.length() - 3) + "ll";
    std::string fileAsm = file.substr(0, file.length() - 3) + "asm";
    std::string fileOut = file.substr(0, file.length() - 4);

    // help command
    if (optionsMenu->getOptions().at(4)->getUsed()) {
        optionsMenu->print();
        exit(0);
    }

    // sem and compile
    optionsMenu->getStmtList()->sem();
    std::cout <<"SEM COMPLETE\n"; std::cout.flush();
    // pseudoST.printST();
    if (semError) exit(1);
    optionsMenu->getStmtList()->llvm_compile_and_dump(fileLL, optionsMenu->getOptions().at(0)->getUsed());

    // compile ir to asm
    std::string command = "clang " + fileLL + " -o " + fileAsm + " -S";
    if (std::system(command.c_str()) == -1) {
        std::cout << "There was an error compiling IR to Assembly";
        exit(1);
    }

    // compile asm to exe
    command = "clang -o " + fileOut + " " + fileLL + " library/lib.a -lm";
    if (std::system(command.c_str()) == -1) {
        std::cout << "There was an error compiling Assembly to Executable";
        exit(1);
    }
    
    // print AST
    if (optionsMenu->getOptions().at(1)->getUsed()) {
        optionsMenu->getStmtList()->printOn(std::cout);
        print = true;
    }

    // print IR code
    if (optionsMenu->getOptions().at(2)->getUsed()) {
        std::ifstream f(fileLL);
        if (f.is_open()) std::cout << f.rdbuf();
        print = true;
    }

    // print Assembly code
    if (optionsMenu->getOptions().at(3)->getUsed()) {
        std::ifstream f(fileAsm);
        if (f.is_open()) std::cout << f.rdbuf();
        print = true;
    }

    if (!print) {
        /* [TODO] need to redirect stdin, stdout to exe to run properly */
        if (std::system(fileOut.c_str()) == -1) {
            std::cout << "There was an error while running the Executable\n";
            exit(1);
        }
    }

}

void OptionsMenu::print() {
    
    std::cout << "Options:\n"; 
    for (Option *o : options) {
        std::cout << "Flag: " << o->getFlag() << " | " << "Description: " << o->getDescription() << std::endl;
    }
}

OptionsMenu *optionsMenu = new OptionsMenu();