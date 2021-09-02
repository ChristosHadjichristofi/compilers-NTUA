#include "ast.hpp"

llvm::LLVMContext AST::TheContext;
llvm::IRBuilder<> AST::Builder(TheContext);
std::unique_ptr<llvm::Module> AST::TheModule;
std::unique_ptr<llvm::legacy::FunctionPassManager> AST::TheFPM;

llvm::GlobalVariable *AST::TheNL;
llvm::Function *AST::TheWriteInteger;
llvm::Function *AST::TheWriteString;

llvm::Type *AST::i8;
llvm::Type *AST::i32;
llvm::Type *AST::i64;