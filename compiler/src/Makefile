.PHONY: default clean distclean

LLVMCONFIG=llvm-config
CXX=c++
CXXFLAGS=-Wall -std=c++11 -g `$(LLVMCONFIG) --cxxflags`
LDFLAGS=`$(LLVMCONFIG) --ldflags --system-libs --libs all`

default: llama

lexer/lexer.cpp: lexer/lexer.l
	flex -s -o lexer/lexer.cpp lexer/lexer.l

parser/parser.hpp parser/parser.cpp: parser/parser.y
	bison --debug -dv -o parser/parser.cpp parser/parser.y

lexer/lexer.o: lexer/lexer.cpp lexer/lexer.hpp parser/parser.hpp

parser/parser.o: parser/parser.cpp parser/parser.hpp

ast/ast.o: ast/ast.cpp ast/ast.hpp

ast/printOn.o: ast/printOn.cpp ast/ast.hpp

ast/sem.o: ast/sem.cpp ast/ast.hpp

library/library.o: library/library.cpp library/library.hpp

symbol/symbol.o: symbol/symbol.cpp symbol/symbol.hpp

types/types.o: types/types.cpp types/types.hpp

error/error.o: error/error.hpp error/error.cpp

ast/compile.o: ast/ast.hpp ast/compile.cpp

llama: lexer/lexer.o parser/parser.o ast/ast.o ast/sem.o ast/printOn.o ast/compile.o symbol/symbol.o library/library.o types/types.o error/error.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) lexer/lexer.cpp parser/parser.cpp parser/parser.hpp parser/parser.output */*.o *.ll *.s *.asm *.out llama

distclean: clean
	$(RM) llama