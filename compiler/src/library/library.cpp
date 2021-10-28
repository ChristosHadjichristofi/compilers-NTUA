#include "library.hpp"

/************************************/
/*              LIBRARY             */
/************************************/

std::set<std::string> libraryVars = {};

Library::Library() {}

void Library::init() {
    /* print_int : int -> unit */
    Function *tempF = new Function(new Unit());
    tempF->params.push_back(new Integer());
    st.insert("print_int", tempF, ENTRY_FUNCTION);
    SymbolEntry *tempEntry = st.lookup("print_int");
    tempEntry->params.push_back(new SymbolEntry(new Integer()));
    libraryVars.insert("print_int");

    /* print_bool : bool -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Boolean());
    st.insert("print_bool", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("print_bool");
    tempEntry->params.push_back(new SymbolEntry(new Boolean()));
    libraryVars.insert("print_bool");

    /* print_char : char -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Character());
    st.insert("print_char", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("print_char");
    tempEntry->params.push_back(new SymbolEntry(new Character()));
    libraryVars.insert("print_char");

    /* print_float : float -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Float());
    st.insert("print_float", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("print_float");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("print_float");

    /* print_string : string -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Array(new Character(), 1)); 
    st.insert("print_string", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("print_string");
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    libraryVars.insert("print_string");

    /* read_int : unit -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Unit());
    st.insert("read_int", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("read_int");
    tempEntry->params.push_back(new SymbolEntry(new Unit()));
    libraryVars.insert("read_int");

    /* read_bool : unit -> bool */
    tempF = new Function(new Boolean());
    tempF->params.push_back(new Unit());
    st.insert("read_bool", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("read_bool");
    tempEntry->params.push_back(new SymbolEntry(new Unit()));
    libraryVars.insert("read_bool");

    /* read_char : unit -> char */
    tempF = new Function(new Character());
    tempF->params.push_back(new Unit());
    st.insert("read_char", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("read_char");
    tempEntry->params.push_back(new SymbolEntry(new Unit()));
    libraryVars.insert("read_char");

    /* read_float : unit -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Unit());
    st.insert("read_float", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("read_float");
    tempEntry->params.push_back(new SymbolEntry(new Unit()));
    libraryVars.insert("read_float");

    /* read_string : string -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Array(new Character(), 1)); 
    st.insert("read_string", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("read_string");
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    libraryVars.insert("read_string");

    /* abs : int -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Integer());
    st.insert("abs", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("abs");
    tempEntry->params.push_back(new SymbolEntry(new Integer()));
    libraryVars.insert("abs");

    /* fabs : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("fabs", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("fabs");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("fabs");

    /* sqrt : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("sqrt", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("sqrt");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("sqrt");

    /* sin : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("sin", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("sin");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("sin");

    /* cos : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("cos", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("cos");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("cos");

    /* tan : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("tan", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("tan");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("tan");

    /* atan : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("atan", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("atan");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("atan");

    /* exp : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("exp", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("exp");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("exp");

    /* ln : float -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Float());
    st.insert("ln", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("ln");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("ln");

    /* pi : unit -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Unit());
    st.insert("pi", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("pi");
    tempEntry->params.push_back(new SymbolEntry(new Unit()));
    libraryVars.insert("pi");

    /* incr : int ref -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Reference(new Integer()));
    st.insert("incr", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("incr");
    tempEntry->params.push_back(new SymbolEntry(new Reference(new Integer())));
    libraryVars.insert("incr");

    /* decr : int ref -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Reference(new Integer()));
    st.insert("decr", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("decr");
    tempEntry->params.push_back(new SymbolEntry(new Reference(new Integer())));
    libraryVars.insert("decr");

    /* float_of_int : int -> float */
    tempF = new Function(new Float());
    tempF->params.push_back(new Integer());
    st.insert("float_of_int", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("float_of_int");
    tempEntry->params.push_back(new SymbolEntry(new Integer()));
    libraryVars.insert("float_of_int");

    /* int_of_float : float -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Float());
    st.insert("int_of_float", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("int_of_float");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("int_of_float");

    /* round : float -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Float());
    st.insert("round", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("round");
    tempEntry->params.push_back(new SymbolEntry(new Float()));
    libraryVars.insert("round");

    /* int_of_char : char -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Character());
    st.insert("int_of_char", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("int_of_char");
    tempEntry->params.push_back(new SymbolEntry(new Character()));
    libraryVars.insert("int_of_char");

    /* char_of_int : int -> char */
    tempF = new Function(new Character());
    tempF->params.push_back(new Integer());
    st.insert("char_of_int", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("char_of_int");
    tempEntry->params.push_back(new SymbolEntry(new Integer()));
    libraryVars.insert("char_of_int");

    /* strlen : string -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Array(new Character(), 1));
    st.insert("strlen", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("strlen");
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    libraryVars.insert("strlen");

    /* strcmp : string -> string -> int */
    tempF = new Function(new Integer());
    tempF->params.push_back(new Array(new Character(), 1));
    tempF->params.push_back(new Array(new Character(), 1));
    st.insert("strcmp", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("strcmp");
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    libraryVars.insert("strcmp");

    /* strcpy : string -> string -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Array(new Character(), 1));
    tempF->params.push_back(new Array(new Character(), 1));
    st.insert("strcpy", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("strcpy");
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    libraryVars.insert("strcpy");

    /* strcat : string -> string -> unit */
    tempF = new Function(new Unit());
    tempF->params.push_back(new Array(new Character(), 1));
    tempF->params.push_back(new Array(new Character(), 1));
    st.insert("strcat", tempF, ENTRY_FUNCTION);
    tempEntry = st.lookup("strcat");
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    libraryVars.insert("strcat");
}