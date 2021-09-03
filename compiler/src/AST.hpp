#ifndef __AST_CLASS_HPP__
#define __AST_CLASS_HPP__

#include <iostream>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>

#define SHOW_MEM            false
#define SHOW_LINE_MACRO     true

# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
typedef struct YYLTYPE {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} YYLTYPE;

static bool semError = false;

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
    virtual llvm::Value* compile() const = 0;
    void llvm_compile_and_dump(bool optimize = false) {
        // Initialize
        TheModule = std::make_unique<llvm::Module>("llama program", TheContext);
        TheFPM = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());
        if (optimize) {
            TheFPM->add(llvm::createPromoteMemoryToRegisterPass());
            TheFPM->add(llvm::createInstructionCombiningPass());
            TheFPM->add(llvm::createReassociatePass());
            TheFPM->add(llvm::createGVNPass());
            TheFPM->add(llvm::createCFGSimplificationPass());
        }
        TheFPM->doInitialization();
        
        // Initialize types
        i8 = llvm::IntegerType::get(TheContext, 8);
        i32 = llvm::IntegerType::get(TheContext, 32);
        i64 = llvm::IntegerType::get(TheContext, 64);
        
        // Initialize global variables
        llvm::ArrayType *nl_type = llvm::ArrayType::get(i8, 2);
        TheNL = new llvm::GlobalVariable(
        *TheModule, nl_type, true, llvm::GlobalValue::PrivateLinkage,
        llvm::ConstantArray::get(nl_type, {c8('\n'), c8('\0')}), "nl");
        TheNL->setAlignment(llvm::MaybeAlign(1));
        
        // Initialize library functions
        llvm::FunctionType *writeInteger_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext),/*std::vector<llvm::Type *>*/ {i64}, false);
        TheWriteInteger =
        llvm::Function::Create(writeInteger_type, llvm::Function::ExternalLinkage,
                        "writeInteger", TheModule.get());

        llvm::FunctionType *writeString_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext),
                            {llvm::PointerType::get(i8, 0)}, false);
        TheWriteString =
        llvm::Function::Create(writeString_type, llvm::Function::ExternalLinkage,
                        "writeString", TheModule.get());
        
        // Define and start the main function.
        llvm::FunctionType *main_type = llvm::FunctionType::get(i32, {}, false);
        llvm::Function *main =
        llvm::Function::Create(main_type, llvm::Function::ExternalLinkage,
                        "main", TheModule.get());
        llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", main);
        Builder.SetInsertPoint(BB);
        
        // Emit the program code.
        compile();
        Builder.CreateRet(c32(0));
        
        // Verify the IR.
        bool bad = verifyModule(*TheModule, &llvm::errs());
        if (bad) {
            std::cerr << "The IR is bad!" << std::endl;
            TheModule->print(llvm::errs(), nullptr);
            std::exit(1);
        }
        
        // Optimize!
        TheFPM->run(*main);
        
        // Print out the IR.
        TheModule->print(llvm::outs(), nullptr);
    }

protected:
    static llvm::LLVMContext TheContext;
    static llvm::IRBuilder<> Builder;
    static std::unique_ptr<llvm::Module> TheModule;
    static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

    static llvm::GlobalVariable *TheNL;
    static llvm::Function *TheWriteInteger;
    static llvm::Function *TheWriteString;

    static llvm::Type *i8;
    static llvm::Type *i32;
    static llvm::Type *i64;

    static llvm::ConstantInt* c1(int c) {
        return llvm::ConstantInt::get(TheContext, llvm::APInt(1, c, true));
    }
    static llvm::ConstantInt* c8(char c) {
        return llvm::ConstantInt::get(TheContext, llvm::APInt(8, c, true));
    }
    static llvm::ConstantInt* c32(int n) {
        return llvm::ConstantInt::get(TheContext, llvm::APInt(32, n, true));
    }
    static llvm::ConstantInt* c64(int n) {
        return llvm::ConstantInt::get(TheContext, llvm::APInt(64, n, true));
    }
    static llvm::ConstantFP* fp(float n){
        return llvm::ConstantFP::get(TheContext, llvm::APFloat(n));
    }
};

#endif