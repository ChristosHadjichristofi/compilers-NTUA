#include "ast.hpp"

llvm::LLVMContext AST::TheContext;
llvm::IRBuilder<> AST::Builder(TheContext);
std::unique_ptr<llvm::Module> AST::TheModule;
std::unique_ptr<llvm::legacy::FunctionPassManager> AST::TheFPM;

llvm::GlobalVariable *AST::TheNL;
llvm::Function *AST::TheWriteInteger;
llvm::Function *AST::TheWriteBoolean;
llvm::Function *AST::TheWriteChar;
llvm::Function *AST::TheWriteReal;
llvm::Function *AST::TheWriteString;
llvm::Function *AST::TheReadInteger;
llvm::Function *AST::TheReadBoolean;
llvm::Function *AST::TheReadChar;
llvm::Function *AST::TheReadReal;
llvm::Function *AST::TheReadString;
llvm::Function *AST::TheAbs;
llvm::Function *AST::TheFabs;
llvm::Function *AST::TheStringLength;
llvm::Function *AST::TheStringCompare;
llvm::Function *AST::TheStringCopy;
llvm::Function *AST::TheStringConcat;

llvm::Type *AST::i1;
llvm::Type *AST::i8;
llvm::Type *AST::i32;
llvm::Type *AST::i64;
llvm::Type *AST::DoubleTyID;