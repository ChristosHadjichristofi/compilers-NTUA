.PHONY: default clean distclean
CXX=c++
CXXFLAGS=-Wall -std=c++11

default: compiler

lexer.cpp: lexer.l
	flex -s -o lexer.cpp lexer.l

parser.hpp parser.cpp: parser.y
	bison --debug -dv -o parser.cpp parser.y

# Add your dependencies here, e.g.
# parser.o: parser.cpp lexer.hpp symbol.hpp

compiler: lexer.o parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) compiler