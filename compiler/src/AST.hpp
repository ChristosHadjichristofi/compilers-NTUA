#ifndef __AST_CLASS_HPP__
#define __AST_CLASS_HPP__

#include <iostream>

class AST {
public:
    virtual void printOn(std::ostream &out) const = 0;
    virtual ~AST() {}
};

#endif