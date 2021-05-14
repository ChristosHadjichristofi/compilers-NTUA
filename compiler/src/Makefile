.PHONY: default clean distclean
CXX=c++
CXXFLAGS=-Wall -std=c++11

default: compiler

lexer.cpp: lexer.l
	flex -s -o lexer.cpp lexer.l

lexer.o: lexer.cpp lexer.hpp parser.hpp ast.hpp

lexer: lexer.o

parser.hpp parser.cpp: parser.y
	bison --debug -dv -o parser.cpp parser.y

parser.o: parser.cpp lexer.hpp ast.hpp types.hpp AST.hpp

compiler: lexer.o parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) compiler