#ifndef __AST_CLASS_HPP__
#define __AST_CLASS_HPP__

#include <iostream>

#define SHOW_MEM false

# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
typedef struct YYLTYPE {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} YYLTYPE;

class AST {
public:
    struct YYLTYPE {
        int first_line;
        int first_column;
        int last_line;
        int last_column;
    } YYLTYPE;
    virtual void printOn(std::ostream &out) const = 0;
    virtual ~AST() {}
    virtual void sem() {}
};

#endif