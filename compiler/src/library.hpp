#ifndef __LIBRARY_HPP__
#define __LIBRARY_HPP__

#include "symbol.hpp"

class Library {
public:  
    Library() {}
    void init() {
        Function *tempF = new Function(new Unit());
        tempF->params.push_back(new Integer()); 
        st.insert("print_int", tempF, ENTRY_FUNCTION);

        *tempF = new Function(new Unit());
        tempF->params.push_back(new String()); 
        st.insert("print_string", tempF, ENTRY_FUNCTION);
    }
};


#endif
