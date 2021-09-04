#ifndef __AST_CLASS_HPP__
#define __AST_CLASS_HPP__

#include <iostream>
#include <fstream>

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
        i1 = llvm::IntegerType::get(TheContext, 1);
        i8 = llvm::IntegerType::get(TheContext, 8);
        i32 = llvm::IntegerType::get(TheContext, 32);
        i64 = llvm::IntegerType::get(TheContext, 64);
        DoubleTyID = llvm::Type::getX86_FP80Ty(TheContext);
        
        // Initialize global variables
        llvm::ArrayType *nl_type = llvm::ArrayType::get(i8, 2);
        TheNL = new llvm::GlobalVariable(
        *TheModule, nl_type, true, llvm::GlobalValue::PrivateLinkage,
        llvm::ConstantArray::get(nl_type, {c8('\n'), c8('\0')}), "nl");
        TheNL->setAlignment(llvm::MaybeAlign(1));
        
        // Initialize library functions
        /* print_int */
        llvm::FunctionType *writeInteger_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i64 }, false);
        TheWriteInteger =
        llvm::Function::Create(writeInteger_type, llvm::Function::ExternalLinkage,
                        "writeInteger", TheModule.get());
        /* print_bool */
        llvm::FunctionType *writeBoolean_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i1 }, false);
        TheWriteBoolean =
        llvm::Function::Create(writeBoolean_type, llvm::Function::ExternalLinkage,
                       "writeBoolean", TheModule.get());
        /* print_char */
        llvm::FunctionType *writeChar_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i8 }, false);
        TheWriteChar =
        llvm::Function::Create(writeChar_type, llvm::Function::ExternalLinkage,
                       "writeChar", TheModule.get());
        /* print_float */
        llvm::FunctionType *writeReal_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { DoubleTyID }, false);
        TheWriteReal =
        llvm::Function::Create(writeReal_type, llvm::Function::ExternalLinkage,
                       "writeReal", TheModule.get());
        /* print_string */
        llvm::FunctionType *writeString_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext),
                            { llvm::PointerType::get(i8, 0) }, false);
        TheWriteString =
        llvm::Function::Create(writeString_type, llvm::Function::ExternalLinkage,
                        "writeString", TheModule.get());
        /* read_int */
        llvm::FunctionType *readInteger_type =
        llvm::FunctionType::get(i64, { }, false);
        TheReadInteger =
        llvm::Function::Create(readInteger_type, llvm::Function::ExternalLinkage,
                       "readInteger", TheModule.get());
        /* read_bool */
        llvm::FunctionType *readBoolean_type =
        llvm::FunctionType::get(i1, { }, false);
        TheReadBoolean =
        llvm::Function::Create(readBoolean_type, llvm::Function::ExternalLinkage,
                       "readBoolean", TheModule.get());
        /* read_char */
        llvm::FunctionType *readChar_type =
        llvm::FunctionType::get(i8, { }, false);
        TheReadChar =
        llvm::Function::Create(readChar_type, llvm::Function::ExternalLinkage,
                       "readChar", TheModule.get());
        /* read_float */
        llvm::FunctionType *readReal_type =
        llvm::FunctionType::get(DoubleTyID, { }, false);
        TheReadReal =
        llvm::Function::Create(readReal_type, llvm::Function::ExternalLinkage,
                       "readReal", TheModule.get());
        /* read_string */
        llvm::FunctionType *readString_type =
        llvm::FunctionType::get(llvm::PointerType::get(i8, 0), { }, false);
        TheReadString =
        llvm::Function::Create(readString_type, llvm::Function::ExternalLinkage,
                       "readString", TheModule.get());

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
        
        // redirect code to a.ll
        std::string str;
        llvm::raw_string_ostream OS(str);
        OS << *TheModule;
        OS.flush();

        std::ofstream out("a.ll");
        out << str;
        out.close();
        
        // Print out the IR.
        // TheModule->print(llvm::outs(), nullptr);
    }

protected:
    static llvm::LLVMContext TheContext;
    static llvm::IRBuilder<> Builder;
    static std::unique_ptr<llvm::Module> TheModule;
    static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

    static llvm::GlobalVariable *TheNL;
    static llvm::Function *TheWriteInteger;
    static llvm::Function *TheWriteBoolean;
    static llvm::Function *TheWriteChar;   
    static llvm::Function *TheWriteReal;
    static llvm::Function *TheWriteString;
    static llvm::Function *TheReadInteger;
    static llvm::Function *TheReadBoolean;
    static llvm::Function *TheReadChar;
    static llvm::Function *TheReadReal;
    static llvm::Function *TheReadString;

    static llvm::Type *i1;
    static llvm::Type *i8;
    static llvm::Type *i32;
    static llvm::Type *i64;
    static llvm::Type *DoubleTyID;

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
    static llvm::Constant* fp(float f) {
        return llvm::ConstantFP::get(llvm::Type::getX86_FP80Ty(TheContext), f);
    }
};

#endif