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

extern bool semError;
extern std::set<std::string> libraryVars;
extern std::vector<std::pair<std::string, std::set<std::string> > > allFreeVars;
extern void printFreeVars();

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
    virtual llvm::Value* compile() { return nullptr; }
    virtual std::set<std::string> preCompile() { return {}; }
    void llvm_compile_and_dump(std::string file, bool optimize = false) {
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
        
        /* create unit struct (type opaque -> no body) */
        std::string unitName = "unit";
        llvm::StructType *unitType = llvm::StructType::create(TheContext, unitName);
        std::vector<llvm::Type *> emptyBody;
        // emptyBody.push_back(i1);
        unitType->setBody(emptyBody);

        /* create string struct type */
        std::vector<llvm::Type *> members;
        /* ptr to array */
        members.push_back(llvm::PointerType::getUnqual(i8));
        /* dimensions number of array */
        members.push_back(i32);

        /* string is defined as an array of one dim */
        members.push_back(i32);

        /* create the struct */
        std::string arrName = "Array_Character_1";
        llvm::StructType *arrayStruct = llvm::StructType::create(TheContext, arrName);
        arrayStruct->setBody(members);

        // Initialize global variables
        llvm::ArrayType *nl_type = llvm::ArrayType::get(i8, 2);
        TheNL = new llvm::GlobalVariable(
        *TheModule, nl_type, true, llvm::GlobalValue::PrivateLinkage,
        llvm::ConstantArray::get(nl_type, {c8('\n'), c8('\0')}), "nl");
        TheNL->setAlignment(llvm::MaybeAlign(1));
        
        // Initialize library functions
        /* writeInteger - lib.a */
        llvm::FunctionType *writeInteger_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i32 }, false);
        TheWriteInteger =
        llvm::Function::Create(writeInteger_type, llvm::Function::ExternalLinkage,
                        "writeInteger", TheModule.get());
        /* print_int */
        llvm::FunctionType *printInt_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { i32 }, false);
        ThePrintIntInternal =
        llvm::Function::Create(printInt_type, llvm::Function::InternalLinkage,
                       "print_int", TheModule.get());
        llvm::BasicBlock *ThePrintIntBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", ThePrintIntInternal);
        Builder.SetInsertPoint(ThePrintIntBB);
        Builder.CreateCall(TheWriteInteger, { ThePrintIntInternal->getArg(0) });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*ThePrintIntInternal);
        /* writeBoolean - lib.a */
        llvm::FunctionType *writeBoolean_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i1 }, false);
        TheWriteBoolean =
        llvm::Function::Create(writeBoolean_type, llvm::Function::ExternalLinkage,
                       "writeBoolean", TheModule.get());
        /* print_bool */
        llvm::FunctionType *printBool_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { i1 }, false);
        ThePrintBoolInternal =
        llvm::Function::Create(printBool_type, llvm::Function::InternalLinkage,
                       "print_bool", TheModule.get());
        llvm::BasicBlock *ThePrintBoolBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", ThePrintBoolInternal);
        Builder.SetInsertPoint(ThePrintBoolBB);
        Builder.CreateCall(TheWriteBoolean, { ThePrintBoolInternal->getArg(0) });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*ThePrintBoolInternal);
        /* writeChar - lib.a */
        llvm::FunctionType *writeChar_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i8 }, false);
        TheWriteChar =
        llvm::Function::Create(writeChar_type, llvm::Function::ExternalLinkage,
                       "writeChar", TheModule.get());
        /* print_char */
        llvm::FunctionType *printChar_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { i8 }, false);
        ThePrintCharInternal =
        llvm::Function::Create(printChar_type, llvm::Function::InternalLinkage,
                       "print_char", TheModule.get());
        llvm::BasicBlock *ThePrintCharBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", ThePrintCharInternal);
        Builder.SetInsertPoint(ThePrintCharBB);
        Builder.CreateCall(TheWriteChar, { ThePrintCharInternal->getArg(0) });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*ThePrintCharInternal);
        /* writeReal - lib.a */
        llvm::FunctionType *writeReal_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { DoubleTyID }, false);
        TheWriteReal =
        llvm::Function::Create(writeReal_type, llvm::Function::ExternalLinkage,
                       "writeReal", TheModule.get());
        /* print_float */
        llvm::FunctionType *printFloat_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { DoubleTyID }, false);
        ThePrintFloatInternal =
        llvm::Function::Create(printFloat_type, llvm::Function::InternalLinkage,
                       "print_float", TheModule.get());
        llvm::BasicBlock *ThePrintFloatBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", ThePrintFloatInternal);
        Builder.SetInsertPoint(ThePrintFloatBB);
        Builder.CreateCall(TheWriteReal, { ThePrintFloatInternal->getArg(0) });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*ThePrintFloatInternal);
        /* writeString - lib.a */
        llvm::FunctionType *writeString_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext),
                            { llvm::PointerType::get(i8, 0) }, false);
        TheWriteString =
        llvm::Function::Create(writeString_type, llvm::Function::ExternalLinkage,
                        "writeString", TheModule.get());
        /* print_string */
        llvm::FunctionType *printString_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { TheModule->getTypeByName("Array_Character_1")->getPointerTo() }, false);
        ThePrintStringInternal =
        llvm::Function::Create(printString_type, llvm::Function::InternalLinkage,
                       "print_string", TheModule.get());
        llvm::BasicBlock *ThePrintStringBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", ThePrintStringInternal);
        Builder.SetInsertPoint(ThePrintStringBB);
        llvm::Value *printstr_strPtr = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), ThePrintStringInternal->getArg(0), { c32(0), c32(0) }, "stringPtr"));
        Builder.CreateCall(TheWriteString, { printstr_strPtr });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*ThePrintStringInternal);
        /* readInteger - lib.a */
        llvm::FunctionType *readInteger_type =
        llvm::FunctionType::get(i32, false);
        TheReadInteger =
        llvm::Function::Create(readInteger_type, llvm::Function::ExternalLinkage,
                       "readInteger", TheModule.get());
        /* read_int */
        llvm::FunctionType *readInt_type = 
        llvm::FunctionType::get(i32, { TheModule->getTypeByName("unit") }, false);
        TheReadIntInternal =
        llvm::Function::Create(readInt_type, llvm::Function::InternalLinkage,
                       "read_int", TheModule.get());
        llvm::BasicBlock *TheReadIntBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheReadIntInternal);
        Builder.SetInsertPoint(TheReadIntBB);
        Builder.CreateRet(Builder.CreateCall(TheReadInteger));
        TheFPM->run(*TheReadIntInternal);
        /* readBoolean - lib.a */
        llvm::FunctionType *readBoolean_type =
        llvm::FunctionType::get(i1, false);
        TheReadBoolean =
        llvm::Function::Create(readBoolean_type, llvm::Function::ExternalLinkage,
                       "readBoolean", TheModule.get());
        /* read_bool */
        llvm::FunctionType *readBool_type = 
        llvm::FunctionType::get(i1, { TheModule->getTypeByName("unit") }, false);
        TheReadBoolInternal =
        llvm::Function::Create(readBool_type, llvm::Function::InternalLinkage,
                       "read_bool", TheModule.get());
        llvm::BasicBlock *TheReadBoolBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheReadBoolInternal);
        Builder.SetInsertPoint(TheReadBoolBB);
        Builder.CreateRet(Builder.CreateCall(TheReadBoolean));
        TheFPM->run(*TheReadBoolInternal);
        /* readChar - lib.a */
        llvm::FunctionType *readCharacter_type =
        llvm::FunctionType::get(i8, false);
        TheReadChar =
        llvm::Function::Create(readCharacter_type, llvm::Function::ExternalLinkage,
                       "readChar", TheModule.get());
        /* read_char */
        llvm::FunctionType *readChar_type = 
        llvm::FunctionType::get(i8, { TheModule->getTypeByName("unit") }, false);
        TheReadCharInternal =
        llvm::Function::Create(readChar_type, llvm::Function::InternalLinkage,
                       "read_char", TheModule.get());
        llvm::BasicBlock *TheReadCharBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheReadCharInternal);
        Builder.SetInsertPoint(TheReadCharBB);
        Builder.CreateRet(Builder.CreateCall(TheReadChar));
        TheFPM->run(*TheReadCharInternal);
        /* readReal */
        llvm::FunctionType *readReal_type =
        llvm::FunctionType::get(DoubleTyID, false);
        TheReadReal =
        llvm::Function::Create(readReal_type, llvm::Function::ExternalLinkage,
                       "readReal", TheModule.get());
        /* read_float */
        llvm::FunctionType *readFloat_type = 
        llvm::FunctionType::get(DoubleTyID, { TheModule->getTypeByName("unit") }, false);
        TheReadRealInternal =
        llvm::Function::Create(readFloat_type, llvm::Function::InternalLinkage,
                       "read_float", TheModule.get());
        llvm::BasicBlock *TheReadRealBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheReadRealInternal);
        Builder.SetInsertPoint(TheReadRealBB);
        Builder.CreateRet(Builder.CreateCall(TheReadReal));
        TheFPM->run(*TheReadRealInternal);
        /* readString - lib.a */
        llvm::FunctionType *readString_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext),
                            { i32, llvm::PointerType::get(i8, 0) }, false);
        TheReadString =
        llvm::Function::Create(readString_type, llvm::Function::ExternalLinkage,
                       "readString", TheModule.get());
        /* read_string */
        llvm::FunctionType *readStringInternal_type =
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { TheModule->getTypeByName("Array_Character_1")->getPointerTo() }, false);
        TheReadStringInternal =
        llvm::Function::Create(readStringInternal_type, llvm::Function::InternalLinkage,
                       "read_string", TheModule.get());
        llvm::BasicBlock *TheReadStringInternalBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheReadStringInternal);
        Builder.SetInsertPoint(TheReadStringInternalBB);
        llvm::Value *strPtr = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheReadStringInternal->getArg(0), { c32(0), c32(0) }, "stringPtr"));
        llvm::Value *sizePtr = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheReadStringInternal->getArg(0), { c32(0), c32(2) }, "sizePtr"));
        Builder.CreateCall(TheReadString, { sizePtr, strPtr });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*TheReadStringInternal);
        /* abs */
        llvm::FunctionType *abs_type =
        llvm::FunctionType::get(i32, std::vector<llvm::Type *> { i32 }, false);
        TheAbs =
        llvm::Function::Create(abs_type, llvm::Function::ExternalLinkage,
                       "abs", TheModule.get());
        /* fabs */
        llvm::FunctionType *fabs_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheFabs =
        llvm::Function::Create(fabs_type, llvm::Function::ExternalLinkage,
                        "fabs", TheModule.get());
        /* sqrt */
        llvm::FunctionType *sqrt_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheSqrt =
        llvm::Function::Create(sqrt_type, llvm::Function::ExternalLinkage,
                        "sqrt", TheModule.get());
        /* sin */
        llvm::FunctionType *sin_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheSin =
        llvm::Function::Create(sin_type, llvm::Function::ExternalLinkage,
                        "sin", TheModule.get());
        /* cos */
        llvm::FunctionType *cos_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheCos =
        llvm::Function::Create(cos_type, llvm::Function::ExternalLinkage,
                        "cos", TheModule.get());
        /* tan */
        llvm::FunctionType *tan_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheTan =
        llvm::Function::Create(tan_type, llvm::Function::ExternalLinkage,
                        "tan", TheModule.get());
        /* arctan */
        llvm::FunctionType *arctan_type =
        llvm::FunctionType::get(DoubleTyID,
                            std::vector<llvm::Type *> { DoubleTyID }, false);
        TheAtan =
        llvm::Function::Create(arctan_type, llvm::Function::ExternalLinkage,
                        "atan", TheModule.get());
        /* exp */
        llvm::FunctionType *exp_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheExp =
        llvm::Function::Create(exp_type, llvm::Function::ExternalLinkage,
                        "exp", TheModule.get());
        /* ln */
        llvm::FunctionType *ln_type =
        llvm::FunctionType::get(DoubleTyID,std::vector<llvm::Type *> { DoubleTyID }, false);
        TheLn =
        llvm::Function::Create(ln_type, llvm::Function::ExternalLinkage,
                        "ln", TheModule.get());
        /* pi - lib.a */
        llvm::FunctionType *pi_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { }, false);
        ThePi =
        llvm::Function::Create(pi_type, llvm::Function::ExternalLinkage,
                        "piLib", TheModule.get());
        /* pi */
        llvm::FunctionType *pi_int_type = 
        llvm::FunctionType::get(DoubleTyID, { TheModule->getTypeByName("unit") }, false);
        ThePiInternal =
        llvm::Function::Create(pi_int_type, llvm::Function::InternalLinkage,
                       "pi", TheModule.get());
        llvm::BasicBlock *ThePiBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", ThePiInternal);
        Builder.SetInsertPoint(ThePiBB);
        Builder.CreateRet(Builder.CreateCall(ThePi));
        TheFPM->run(*ThePiInternal);
        /* incr */
        llvm::FunctionType *incr_type =
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { llvm::PointerType::get(i32, 0) }, false);
        TheIncr =
        llvm::Function::Create(incr_type, llvm::Function::InternalLinkage,
                       "incr", TheModule.get());
        llvm::BasicBlock *TheIncrBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheIncr);
        Builder.SetInsertPoint(TheIncrBB);
        llvm::Value *valToIncr = Builder.CreateLoad(TheIncr->getArg(0));
        llvm::Value *valIncreased = Builder.CreateAdd(valToIncr, c32(1));
        Builder.CreateStore(valIncreased, TheIncr->getArg(0));
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*TheIncr);
        /* decr */
        llvm::FunctionType *decr_type =
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { llvm::PointerType::get(i32, 0) }, false);
        TheDecr =
        llvm::Function::Create(decr_type, llvm::Function::InternalLinkage,
                       "decr", TheModule.get());
        llvm::BasicBlock *TheDecrBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheDecr);
        Builder.SetInsertPoint(TheDecrBB);
        llvm::Value *valToDecr = Builder.CreateLoad(TheDecr->getArg(0));
        llvm::Value *valDecreased = Builder.CreateSub(valToDecr, c32(1));
        Builder.CreateStore(valDecreased, TheDecr->getArg(0));
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*TheDecr);
        /* float_of_int */
        llvm::FunctionType *floatOfInt_type =
        llvm::FunctionType::get(DoubleTyID, std::vector<llvm::Type *> { i32 }, false);
        TheFloatOfInt =
        llvm::Function::Create(floatOfInt_type, llvm::Function::InternalLinkage,
                       "float_of_int", TheModule.get());
        llvm::BasicBlock *TheFltToIntBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheFloatOfInt);
        Builder.SetInsertPoint(TheFltToIntBB);
        // https://stackoverflow.com/questions/61293548/understanding-the-llvm-cast-instruction
        llvm::Value *convertedInt = Builder.CreateCast(llvm::Instruction::SIToFP, TheFloatOfInt->getArg(0), DoubleTyID);
        Builder.CreateRet(convertedInt);
        TheFPM->run(*TheFloatOfInt);
        /* trunc - lib.a */
        llvm::FunctionType *trunc_type =
        llvm::FunctionType::get(i32, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheTrunc =
        llvm::Function::Create(trunc_type, llvm::Function::ExternalLinkage,
                       "trunc", TheModule.get());
        /* int_of_float */
        llvm::FunctionType *intOfFloat_type = 
        llvm::FunctionType::get(i32, { DoubleTyID }, false);
        TheIntOfFloat =
        llvm::Function::Create(intOfFloat_type, llvm::Function::InternalLinkage,
                       "int_of_float", TheModule.get());
        llvm::BasicBlock *TheIntOfFloatBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheIntOfFloat);
        Builder.SetInsertPoint(TheIntOfFloatBB);
        Builder.CreateRet(Builder.CreateCall(TheTrunc, { TheIntOfFloat->getArg(0) }));
        TheFPM->run(*TheIntOfFloat);
        /* round */
        llvm::FunctionType *round_Fromlib_type =
        llvm::FunctionType::get(i32, std::vector<llvm::Type *> { DoubleTyID }, false);
        TheRound =
        llvm::Function::Create(round_Fromlib_type, llvm::Function::ExternalLinkage,
                       "round", TheModule.get());
        /* ord - lib.a */
        llvm::FunctionType *ord_type =
        llvm::FunctionType::get(i32, std::vector<llvm::Type *> { i8 }, false);
        TheOrd =
        llvm::Function::Create(ord_type, llvm::Function::ExternalLinkage,
                       "ord", TheModule.get());
        /* int_of_char */
        llvm::FunctionType *intOfChar_type = 
        llvm::FunctionType::get(i32, { i8 }, false);
        TheIntOfChar =
        llvm::Function::Create(intOfChar_type, llvm::Function::InternalLinkage,
                       "int_of_char", TheModule.get());
        llvm::BasicBlock *TheIntOfCharBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheIntOfChar);
        Builder.SetInsertPoint(TheIntOfCharBB);
        Builder.CreateRet(Builder.CreateCall(TheOrd, { TheIntOfChar->getArg(0) }));
        TheFPM->run(*TheIntOfChar);
        /* TheChr */
        llvm::FunctionType *chr_type =
        llvm::FunctionType::get(i8, std::vector<llvm::Type *> { i32 }, false);
        TheChr =
        llvm::Function::Create(chr_type, llvm::Function::ExternalLinkage,
                       "chr", TheModule.get());
        /* char_of_int */
        llvm::FunctionType *charOfInt_type = 
        llvm::FunctionType::get(i8, { i32 }, false);
        TheCharOfInt =
        llvm::Function::Create(charOfInt_type, llvm::Function::InternalLinkage,
                       "char_of_int", TheModule.get());
        llvm::BasicBlock *TheCharOfIntBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheCharOfInt);
        Builder.SetInsertPoint(TheCharOfIntBB);
        Builder.CreateRet(Builder.CreateCall(TheChr, { TheCharOfInt->getArg(0) }));
        TheFPM->run(*TheCharOfInt);
        /* strlen - lib.a */
        llvm::FunctionType *stringLength_type =
        llvm::FunctionType::get(i32, { llvm::PointerType::get(i8, 0) }, false);
        TheStringLength =
        llvm::Function::Create(stringLength_type, llvm::Function::ExternalLinkage,
                       "strlenLib", TheModule.get());
        /* strlen */
        llvm::FunctionType *strlen_type = 
        llvm::FunctionType::get(i32, { TheModule->getTypeByName("Array_Character_1")->getPointerTo() }, false);
        TheStringLengthInternal =
        llvm::Function::Create(strlen_type, llvm::Function::InternalLinkage,
                       "strlen", TheModule.get());
        llvm::BasicBlock *TheStringLengthInternalBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheStringLengthInternal);
        Builder.SetInsertPoint(TheStringLengthInternalBB);
        llvm::Value *strlen_strPtr = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringLengthInternal->getArg(0), { c32(0), c32(0) }, "stringPtr"));
        Builder.CreateRet(Builder.CreateCall(TheStringLength, { strlen_strPtr }));
        TheFPM->run(*TheStringLengthInternal);
        /* strcmp - lib.a */
        llvm::FunctionType *stringCompare_type =
        llvm::FunctionType::get(i32, { llvm::PointerType::get(i8, 0), llvm::PointerType::get(i8, 0) }, false);
        TheStringCompare =
        llvm::Function::Create(stringCompare_type, llvm::Function::ExternalLinkage,
                       "strcmpLib", TheModule.get());
        /* strcmp */
        llvm::FunctionType *strcmp_type = 
        llvm::FunctionType::get(i32, { TheModule->getTypeByName("Array_Character_1")->getPointerTo(), TheModule->getTypeByName("Array_Character_1")->getPointerTo() }, false);
        TheStringCompareInternal =
        llvm::Function::Create(strcmp_type, llvm::Function::InternalLinkage,
                       "strcmp", TheModule.get());
        llvm::BasicBlock *TheStringCompareInternalBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheStringCompareInternal);
        Builder.SetInsertPoint(TheStringCompareInternalBB);
        llvm::Value *strcmp_strPtr1 = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringCompareInternal->getArg(0), { c32(0), c32(0) }, "stringPtr1"));
        llvm::Value *strcmp_strPtr2 = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringCompareInternal->getArg(1), { c32(0), c32(0) }, "stringPtr2"));
        Builder.CreateRet(Builder.CreateCall(TheStringCompare, { strcmp_strPtr1, strcmp_strPtr2 }));
        TheFPM->run(*TheStringCompareInternal);
        /* strcpy - lib.a */
        llvm::FunctionType *stringCopy_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { llvm::PointerType::get(i8, 0), llvm::PointerType::get(i8, 0) }, false);
        TheStringCopy =
        llvm::Function::Create(stringCopy_type, llvm::Function::ExternalLinkage,
                       "strcpyLib", TheModule.get());
        /* strcpy */
        llvm::FunctionType *strcpy_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { TheModule->getTypeByName("Array_Character_1")->getPointerTo(), TheModule->getTypeByName("Array_Character_1")->getPointerTo() }, false);
        TheStringCopyInternal =
        llvm::Function::Create(strcpy_type, llvm::Function::InternalLinkage,
                       "strcpy", TheModule.get());
        llvm::BasicBlock *TheStringCopyInternalBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheStringCopyInternal);
        Builder.SetInsertPoint(TheStringCopyInternalBB);
        llvm::Value *strcpy_strPtr1 = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringCopyInternal->getArg(0), { c32(0), c32(0) }, "stringPtr"));
        llvm::Value *strcpy_strPtr2 = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringCopyInternal->getArg(1), { c32(0), c32(0) }, "stringPtr"));
        Builder.CreateCall(TheStringCopy, { strcpy_strPtr1, strcpy_strPtr2 });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*TheStringCopyInternal);
        /* strcat - lib.a */
        llvm::FunctionType *stringConcat_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { llvm::PointerType::get(i8, 0), llvm::PointerType::get(i8, 0) }, false);
        TheStringConcat =
        llvm::Function::Create(stringConcat_type, llvm::Function::ExternalLinkage,
                       "strcatLib", TheModule.get());
        /* strcat */
        llvm::FunctionType *strcat_type = 
        llvm::FunctionType::get(TheModule->getTypeByName("unit"), { TheModule->getTypeByName("Array_Character_1")->getPointerTo(), TheModule->getTypeByName("Array_Character_1")->getPointerTo() }, false);
        TheStringConcatInternal =
        llvm::Function::Create(strcat_type, llvm::Function::InternalLinkage,
                       "strcat", TheModule.get());
        llvm::BasicBlock *TheStringConcatInternalBB = llvm::BasicBlock::Create(TheModule->getContext(), "entry", TheStringConcatInternal);
        Builder.SetInsertPoint(TheStringConcatInternalBB);
        llvm::Value *strcat_strPtr1 = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringConcatInternal->getArg(0), { c32(0), c32(0) }, "stringPtr1"));
        llvm::Value *strcat_strPtr2 = Builder.CreateLoad(Builder.CreateGEP(TheModule->getTypeByName("Array_Character_1"), TheStringConcatInternal->getArg(1), { c32(0), c32(0) }, "stringPtr2"));
        Builder.CreateCall(TheStringConcat, { strcat_strPtr1, strcat_strPtr2 });
        Builder.CreateRet(llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")));
        TheFPM->run(*TheStringConcatInternal);
        /* exit */
        llvm::FunctionType *exit_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), { i32 }, false);
        TheExit =
        llvm::Function::Create(exit_type, llvm::Function::ExternalLinkage,
                       "exit", TheModule.get());

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

        std::ofstream out(file);
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
    static llvm::Function *ThePrintIntInternal;
    static llvm::Function *TheWriteBoolean;
    static llvm::Function *ThePrintBoolInternal;
    static llvm::Function *TheWriteChar;
    static llvm::Function *ThePrintCharInternal;   
    static llvm::Function *TheWriteReal;
    static llvm::Function *ThePrintFloatInternal;
    static llvm::Function *TheWriteString;
    static llvm::Function *ThePrintStringInternal;
    static llvm::Function *TheReadInteger;
    static llvm::Function *TheReadIntInternal;
    static llvm::Function *TheReadBoolean;
    static llvm::Function *TheReadBoolInternal;
    static llvm::Function *TheReadChar;
    static llvm::Function *TheReadCharInternal;
    static llvm::Function *TheReadReal;
    static llvm::Function *TheReadRealInternal;
    static llvm::Function *TheReadString;
    static llvm::Function *TheReadStringInternal;
    static llvm::Function *TheAbs;
    static llvm::Function *TheFabs;
    static llvm::Function *TheSqrt;
    static llvm::Function *TheSin;
    static llvm::Function *TheCos;
    static llvm::Function *TheTan;
    static llvm::Function *TheAtan;
    static llvm::Function *TheExp;
    static llvm::Function *TheLn;
    static llvm::Function *ThePi;
    static llvm::Function *ThePiInternal;
    static llvm::Function *TheIncr;
    static llvm::Function *TheDecr;
    static llvm::Function *TheFloatOfInt;
    static llvm::Function *TheTrunc;
    static llvm::Function *TheIntOfFloat;
    static llvm::Function *TheRound;
    static llvm::Function *TheOrd;
    static llvm::Function *TheIntOfChar;
    static llvm::Function *TheChr;
    static llvm::Function *TheCharOfInt;
    static llvm::Function *TheStringLength;
    static llvm::Function *TheStringLengthInternal;
    static llvm::Function *TheStringCompare;
    static llvm::Function *TheStringCompareInternal;
    static llvm::Function *TheStringCopy;
    static llvm::Function *TheStringCopyInternal;
    static llvm::Function *TheStringConcat;
    static llvm::Function *TheStringConcatInternal;
    static llvm::Function *TheExit;

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