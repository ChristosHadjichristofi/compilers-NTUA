.PHONY: default clean distclean

LLVMCONFIG=llvm-config
CXX=c++
CXXFLAGS=-Wall -std=c++11 -g `$(LLVMCONFIG) --cxxflags`
LDFLAGS=`$(LLVMCONFIG) --ldflags --system-libs --libs all`

default: llama

lexer/lexer.cpp: lexer/lexer.l
	flex -s -o lexer/lexer.cpp lexer/lexer.l

lexer/lexer.o: lexer/lexer.cpp lexer/lexer.hpp parser/parser.hpp ast/ast.hpp

lexer/lexer: lexer/lexer.o

parser/parser.hpp parser/parser.cpp: parser/parser.y
	bison --debug -dv -o parser/parser.cpp parser/parser.y

parser/parser.o: parser/parser.cpp lexer/lexer.hpp ast/ast.hpp types/types.hpp ast/AST.hpp library/library.hpp symbol/symbol.hpp error/error.hpp

llama: lexer/lexer.o parser/parser.o ast/ast.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) lexer/lexer.cpp parser/parser.cpp parser/parser.hpp parser/parser.output */*.o *.ll *.s *.asm *.out llama

distclean: clean
	$(RM) llama