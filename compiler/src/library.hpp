#ifndef __LIBRARY_HPP__
#define __LIBRARY_HPP__

#include "symbol.hpp"

class Library {
public:  
    Library() {}
    void init() {
        /* print_int : int -> unit */
        Function *tempF = new Function(new Unit());
        tempF->params.push_back(new Integer());
        st.insert("print_int", tempF, ENTRY_FUNCTION);
        SymbolEntry *tempEntry = st.lookup("print_int");
        tempEntry->params.push_back(new SymbolEntry(new Integer()));

        /* print_bool : bool -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Boolean());
        st.insert("print_bool", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("print_bool");
        tempEntry->params.push_back(new SymbolEntry(new Boolean()));

        /* print_char : char -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Character());
        st.insert("print_char", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("print_char");
        tempEntry->params.push_back(new SymbolEntry(new Character()));

        /* print_float : float -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Float());
        st.insert("print_float", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("print_float");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* print_string : string -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Array(new Character(), 1)); 
        st.insert("print_string", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("print_string");
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));



        /* read_int : unit -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Unit());
        st.insert("read_int", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("read_int");
        tempEntry->params.push_back(new SymbolEntry(new Unit()));

        /* read_bool : unit -> bool */
        tempF = new Function(new Boolean());
        tempF->params.push_back(new Unit());
        st.insert("read_bool", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("read_bool");
        tempEntry->params.push_back(new SymbolEntry(new Unit()));

        /* read_char : unit -> char */
        tempF = new Function(new Character());
        tempF->params.push_back(new Unit());
        st.insert("read_char", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("read_char");
        tempEntry->params.push_back(new SymbolEntry(new Unit()));

        /* read_float : unit -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Unit());
        st.insert("read_float", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("read_float");
        tempEntry->params.push_back(new SymbolEntry(new Unit()));

        /* read_string : unit -> string */
        tempF = new Function(new Array(new Character(), 1));
        tempF->params.push_back(new Unit()); 
        st.insert("read_string", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("read_string");
        tempEntry->params.push_back(new SymbolEntry(new Unit()));

        /* abs : int -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Integer());
        st.insert("abs", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("abs");
        tempEntry->params.push_back(new SymbolEntry(new Integer()));

        /* fabs : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("fabs", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("fabs");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* sqrt : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("sqrt", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("sqrt");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* sin : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("sin", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("sin");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* cos : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("cos", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("cos");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* tan : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("tan", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("tan");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* atan : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("atan", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("atan");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* exp : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("exp", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("exp");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* ln : float -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Float());
        st.insert("ln", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("ln");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* pi : unit -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Unit());
        st.insert("pi", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("pi");
        tempEntry->params.push_back(new SymbolEntry(new Unit()));

        /* incr : int ref -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Reference(new Integer()));
        st.insert("incr", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("incr");
        tempEntry->params.push_back(new SymbolEntry(new Reference(new Integer())));

        /* decr : int ref -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Reference(new Integer()));
        st.insert("decr", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("decr");
        tempEntry->params.push_back(new SymbolEntry(new Reference(new Integer())));

        /* float_of_int : int -> float */
        tempF = new Function(new Float());
        tempF->params.push_back(new Integer());
        st.insert("float_of_int", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("float_of_int");
        tempEntry->params.push_back(new SymbolEntry(new Integer()));

        /* int_of_float : float -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Float());
        st.insert("int_of_float", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("int_of_float");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* round : float -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Float());
        st.insert("round", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("round");
        tempEntry->params.push_back(new SymbolEntry(new Float()));

        /* int_of_char : char -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Character());
        st.insert("int_of_char", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("int_of_char");
        tempEntry->params.push_back(new SymbolEntry(new Character()));

        /* char_of_int : int -> char */
        tempF = new Function(new Character());
        tempF->params.push_back(new Integer());
        st.insert("char_of_int", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("char_of_int");
        tempEntry->params.push_back(new SymbolEntry(new Integer()));

        /* strlen : string -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Array(new Character(), 1));
        st.insert("strlen", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("strlen");
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));

        /* strcmp : string -> string -> int */
        tempF = new Function(new Integer());
        tempF->params.push_back(new Array(new Character(), 1));
        tempF->params.push_back(new Array(new Character(), 1));
        st.insert("strcmp", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("strcmp");
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));

        /* strcpy : string -> string -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Array(new Character(), 1));
        tempF->params.push_back(new Array(new Character(), 1));
        st.insert("strcpy", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("strcpy");
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));

        /* strcat : string -> string -> unit */
        tempF = new Function(new Unit());
        tempF->params.push_back(new Array(new Character(), 1));
        tempF->params.push_back(new Array(new Character(), 1));
        st.insert("strcat", tempF, ENTRY_FUNCTION);
        tempEntry = st.lookup("strcat");
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
        tempEntry->params.push_back(new SymbolEntry(new Array(new Character(), 1)));
    }
};


#endif
