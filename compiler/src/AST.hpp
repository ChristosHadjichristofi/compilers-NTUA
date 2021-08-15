#ifndef __AST_CLASS_HPP__
#define __AST_CLASS_HPP__

#include <iostream>

#define SHOW_MEM false

class AST {
public:
    virtual void printOn(std::ostream &out) const = 0;
    virtual ~AST() {}
    virtual void sem() {}
};

#endif