#include "ast.hpp"
#include <vector>
#include <set>
#include "../symbol/symbol.hpp"

/************************************/
/*               EXPR               */
/************************************/

llvm::Value* Expr::compile() { return nullptr; }

/************************************/
/*             PATTERN              */
/************************************/

void Pattern::setMatchExprV(llvm::Value *v) { matchExprV = v; }

void Pattern::setNextClauseBlock(llvm::BasicBlock *bb) { nextClauseBlock = bb; }

/************************************/
/*               BLOCK              */
/************************************/

llvm::Value* Block::compile() {

    pseudoST.initSize();
    currPseudoScope = currPseudoScope->getNext();
    currPseudoScope = currPseudoScope->getNext();
    for (auto i = block.rbegin(); i != block.rend(); ++i) (*i)->compile();
    currPseudoScope = currPseudoScope->getPrev();
    currPseudoScope = currPseudoScope->getPrev();
    return nullptr;
}

llvm::GlobalVariable* Block::createGlobalVariable(llvm::Type *t) {
    auto globalVar = new llvm::GlobalVariable(
        *TheModule,
        t,
        false,
        llvm::GlobalValue::InternalLinkage,
        llvm::ConstantAggregateZero::get(t)
    );
    return globalVar;
}

/************************************/
/*              EXRP GEN            */
/************************************/

llvm::Value* ExprGen::compile() { return expr->compile(); }

/************************************/
/*               ID                 */
/************************************/

llvm::Value* Id::compile() {
    SymbolEntry *se = currPseudoScope->lookup(name, pseudoST.getSize());
    if (expr == nullptr && exprGen == nullptr) {
        if (se != nullptr) {
            if (se->Value != nullptr) return se->Value;
            return TheModule->getFunction(se->id);
        }
        else std::cout << "Couldn't find " << name << std::endl;
    }
    else {
        if (se == nullptr) {
            Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("This is message should never see the light of day\n")) });
            Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
        }

        std::vector<llvm::Value *> args;
        llvm::Value *v = expr->compile();

        args.push_back(v);
        ExprGen *currExpr = exprGen;
        while (currExpr != nullptr) {
            v = currExpr->compile();
            args.push_back(v);
            currExpr = currExpr->getNext();
        }

        if (se->Value != nullptr) return Builder.CreateCall(se->Value, args);
        else return Builder.CreateCall(TheModule->getFunction(name), args);
    }

    /* might need to throw error when reach this point, cause means function does not exist */
    return nullptr;
}

/************************************/
/*            PATTERN ID            */
/************************************/

llvm::Value* PatternId::compile() { pseudoST.incrSize(); return c1(true); }

/************************************/
/*            PATTERN GEN           */
/************************************/

llvm::Value* PatternGen::compile() {
    pattern->setMatchExprV(matchExprV);
    pattern->setNextClauseBlock(nextClauseBlock);
    return pattern->compile();
}

/************************************/
/*          PATTERN CONSTR          */
/************************************/

llvm::Value* PatternConstr::compile() {

    SymbolEntry *se = currPseudoScope->lookup(Id, pseudoST.getSize());

    int patternConstr_tag = 0;
    for (long unsigned int i = 0; i < se->params.front()->params.size(); i++) {
        if (se == se->params.front()->params.at(i)) patternConstr_tag = i;
    }
    
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *EqConstrsBlock = llvm::BasicBlock::Create(TheContext);

    llvm::Value *matchExprV_tag = Builder.CreateLoad(Builder.CreateGEP(matchExprV, { c32(0), c32(0) }), "match_expr");
    llvm::Value *comparison = Builder.CreateICmpEQ(c32(patternConstr_tag), matchExprV_tag);

    Builder.CreateCondBr(comparison, EqConstrsBlock, nextClauseBlock);

    TheFunction->getBasicBlockList().push_back(EqConstrsBlock);
    Builder.SetInsertPoint(EqConstrsBlock);

    llvm::Value *matchExprV_casted = Builder.CreatePointerCast(matchExprV, se->LLVMType->getPointerTo());

    llvm::Value *matched = c1(true);
    PatternGen *tempPatternGen = patternGen;
    while (tempPatternGen != nullptr) {
        if (tempPatternGen->getName().compare("")) pseudoST.incrSize();
        tempPatternGen = tempPatternGen->getNext();
    }
    tempPatternGen = patternGen;

    int index = 0;
    SymbolEntry *tempSE = nullptr;
    while (tempPatternGen != nullptr) {
        std::string tempName = tempPatternGen->getName();

        llvm::Value *temp;
        if (tempName.compare(""))
            temp = Builder.CreateLoad(Builder.CreateGEP(matchExprV_casted, { c32(0), c32(++index) }), tempName);
        else temp = Builder.CreateLoad(Builder.CreateGEP(matchExprV_casted, { c32(0), c32(++index) }));
        /* assign values to local variables (PatternId) */
        if (tempName.compare("")) {
            tempSE = currPseudoScope->lookup(tempName, pseudoST.getSize());
            tempSE->Value = temp;
        }

        tempPatternGen->setMatchExprV(temp);
        tempPatternGen->setNextClauseBlock(nextClauseBlock);
        temp = tempPatternGen->compile();
        
        matched = Builder.CreateAnd(matched, temp);

        tempPatternGen = tempPatternGen->getNext();
    }

    return matched;

}

/************************************/
/*              CLAUSE              */
/************************************/

llvm::Value* Clause::patternCompile() { return pattern->compile(); }

llvm::Value* Clause::compile() { return expr->compile(); }

/************************************/
/*           BAR CLAUSE GEN         */
/************************************/

/* intentionally left empty =) */
llvm::Value* BarClauseGen::compile() { return nullptr; }

/************************************/
/*               MATCH              */
/************************************/

llvm::Value* Match::compile() {

    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    
    /* get expr compile to be matched */
    currPseudoScope = currPseudoScope->getNext();
    llvm::Value *matchExpr = expr->compile();

    /* vector to keep all clauses in order to iterate through it */
    std::vector<Clause *> clauses;
    clauses.push_back(clause);
    
    if (barClauseGen != nullptr) {
        BarClauseGen *tempBCG = barClauseGen;
        while (tempBCG != nullptr) {
            clauses.push_back(tempBCG->getClause());
            tempBCG = tempBCG->getBarClauseGen();
        }
    }

    /* create necessary blocks and block vectors */
    std::vector<llvm::BasicBlock *> clausesBlocks;
    std::vector<llvm::Value *> clausesValues;
    llvm::BasicBlock *SuccessBlock;
    llvm::BasicBlock *NextClauseBlock = llvm::BasicBlock::Create(TheContext);
    llvm::BasicBlock *FinishBlock = llvm::BasicBlock::Create(TheContext);

    Builder.CreateBr(NextClauseBlock);

    for (auto clause : clauses) {

        /* move to next clause block */
        TheFunction->getBasicBlockList().push_back(NextClauseBlock);
        Builder.SetInsertPoint(NextClauseBlock);

        /* create next and success block of clause */
        NextClauseBlock = llvm::BasicBlock::Create(TheContext);
        SuccessBlock = llvm::BasicBlock::Create(TheContext);

        /* move scope as each clause opens a scope */
        currPseudoScope = currPseudoScope->getNext();

        /* set to each clause the expression they are trying to match */
        clause->getPattern()->setMatchExprV(matchExpr);
        /* set to each clause their next clause block */
        clause->getPattern()->setNextClauseBlock(NextClauseBlock);
        
        /* branch in case clause pattern matches the expr pattern */
        Builder.CreateCondBr(clause->patternCompile(), SuccessBlock, NextClauseBlock);

        /* if clause pattern matches the expr pattern */
        TheFunction->getBasicBlockList().push_back(SuccessBlock);
        Builder.SetInsertPoint(SuccessBlock);

        clausesValues.push_back(clause->compile());

        /* move scope back every time a clause finishes */
        currPseudoScope = currPseudoScope->getPrev();

        /* block needed for phi node */
        clausesBlocks.push_back(Builder.GetInsertBlock());

        Builder.CreateBr(FinishBlock);
    }

    /* case that no clause pattern matched the expr pattern */
    TheFunction->getBasicBlockList().push_back(NextClauseBlock);
    Builder.SetInsertPoint(NextClauseBlock);

    Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Match Failure\n")) });
    Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });

    Builder.CreateBr(NextClauseBlock);

    /* finish of match */
    TheFunction->getBasicBlockList().push_back(FinishBlock);
    Builder.SetInsertPoint(FinishBlock);

    llvm::Type *returnTy = clause->getType()->getLLVMType();
    llvm::PHINode *v = Builder.CreatePHI(returnTy, clauses.size());
    for (long unsigned int i = 0; i < clauses.size(); i++) v->addIncoming(clausesValues[i], clausesBlocks[i]);

    currPseudoScope = currPseudoScope->getPrev();

    return v;
}

/************************************/
/*                FOR               */
/************************************/

llvm::Value* For::compile() {
    /* compile start */
    llvm::Value *startValue = start->compile();
    if (startValue == nullptr) return nullptr;

    // Make the new basic block for the loop header, inserting after current
    // block.
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *PreheaderBB = Builder.GetInsertBlock();
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

    // Insert an explicit fall through from the current block to the LoopBB.
    Builder.CreateBr(LoopBB);

    // Start insertion in LoopBB.
    Builder.SetInsertPoint(LoopBB);

    // Start the PHI node with an entry for Start.
    llvm::PHINode *Variable =
    Builder.CreatePHI(llvm::Type::getInt32Ty(TheContext), 2, id);
    Variable->addIncoming(startValue, PreheaderBB);

    // increase size of pseudoST for correct lookup (newly added var of For)
    pseudoST.incrSize();
    // fetch from SymbolTable
    currPseudoScope = currPseudoScope->getNext();
    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
    se->Value = Variable;

    if (se->isFreeVar) {
        se->GlobalValue = createGlobalVariable(se->type->getLLVMType());
        Builder.CreateStore(se->Value, se->GlobalValue);
    }

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB. Note that we ignore the value computed by the body, but don't
    // allow an error.
    if (!expr->compile()) return nullptr;

    // Emit the step value. Not supported, use 1.
    llvm::Value *stepValue = nullptr;
    stepValue = c32(1);

    llvm::Value *nextVar = nullptr;
    if (ascending) nextVar = Builder.CreateAdd(Variable, stepValue, "nextvar");
    else nextVar = Builder.CreateSub(Variable, stepValue, "nextvar");

    // Compute the end condition.
    llvm::Value *endVar = end->compile();
    if (!endVar) return nullptr;

    llvm::Value *endCond = Builder.CreateICmpSLE(nextVar, endVar);

    // Create the "after loop" block and insert it.
    llvm::BasicBlock *LoopEndBB = Builder.GetInsertBlock();
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext, "afterloop", TheFunction);

    // Insert the conditional branch into the end of LoopEndBB.
    Builder.CreateCondBr(endCond, LoopBB, AfterBB);

    // Any new code will be inserted in AfterBB.
    Builder.SetInsertPoint(AfterBB);

    // Add a new entry to the PHI node for the backedge.
    Variable->addIncoming(nextVar, LoopEndBB);

    // close Scope
    currPseudoScope = currPseudoScope->getPrev();

    // for expr always returns 0.
    return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
}

/************************************/
/*              WHILE               */
/************************************/

llvm::Value* While::compile() {
    /* Open Scope */
    currPseudoScope = currPseudoScope->getNext();
    /* get Previous Basic Block and define new ones */
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);
    llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(TheContext, "body", TheFunction);
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext, "endwhile", TheFunction);
    /* Enter Loop */
    Builder.CreateBr(LoopBB);
    Builder.SetInsertPoint(LoopBB);
    /* Evaluate loop condition and create conditional branch */
    llvm::Value *loop_cond = loopCondition->compile();
    Builder.CreateCondBr(loop_cond, BodyBB, AfterBB);
    /* Enter Body */
    Builder.SetInsertPoint(BodyBB);
    /* Emit code for body */
    expr->compile();
    /* Branch to Loop Basic Block - condition will be evaluated there */
    Builder.CreateBr(LoopBB);
    /* Finish While */
    Builder.SetInsertPoint(AfterBB);
    /* Close Scope */
    currPseudoScope = currPseudoScope->getPrev();
    /* Return unit */
    return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
}

/************************************/
/*                IF                */
/************************************/

llvm::Value* If::compile() {
    /* compile condition value and create the condition */
    llvm::Value *condValue = condition->compile();
    llvm::Value *cond = Builder.CreateICmpNE(condValue, c1(false), "if_cond");
    llvm::Value *thenValue = nullptr;
    llvm::Value *elseValue = nullptr;

    /* create basic blocks for then, else and after */
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else", TheFunction);
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext, "endif", TheFunction);

    /* conditional branch, set insert point and save thenValue */
    Builder.CreateCondBr(cond, ThenBB, ElseBB);
    Builder.SetInsertPoint(ThenBB);
    thenValue = expr1->compile();
    ThenBB = Builder.GetInsertBlock();
    /* branch to after block */
    Builder.CreateBr(AfterBB);

    /* set insert point and save elseValue (default is unit) */
    Builder.SetInsertPoint(ElseBB);
    elseValue = llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));

    /* in case that expr2 exists then elseValue must be updated */
    if (expr2 != nullptr) elseValue = expr2->compile();
    ElseBB = Builder.GetInsertBlock();
    /* branch to after block */
    Builder.CreateBr(AfterBB);

    /* insert point to after block, create phi node and connect with then block and else block */
    Builder.SetInsertPoint(AfterBB);

    llvm::PHINode *phiNodeRet = Builder.CreatePHI(thenValue->getType(), 2, "if_ret_val");
    phiNodeRet->addIncoming(thenValue, ThenBB);
    phiNodeRet->addIncoming(elseValue, ElseBB);

    /* return value */
    return phiNodeRet;
}

/************************************/
/*               BEGIN              */
/************************************/

llvm::Value* Begin::compile() {
    currPseudoScope = currPseudoScope->getNext();
    llvm::Value *v = expr->compile();
    currPseudoScope = currPseudoScope->getPrev();
    return v;
}

/************************************/
/*          COMMA EXPR GEN          */
/************************************/

llvm::Value* CommaExprGen::compile() { return expr->compile(); }

/************************************/
/*               PAR                */
/************************************/

llvm::Value* Par::compile() {

    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize()+1);
    if (se != nullptr) {
        pseudoST.incrSize(); // increase size only after veryfying it is in ST
        se->LLVMType = se->type->getLLVMType();

        if (getRefFinalType(se->type).first->typeValue == TYPE_UNKNOWN) {
            if (SHOW_LINE_MACRO) std::cout << "[LINE: " << __LINE__ << "] ";
            std::cout << "Warning at: Line " << YYLTYPE.first_line << ", Characters " << YYLTYPE.first_column << " - " << YYLTYPE.last_column << std::endl;
            Error *err = new Warning(id);
            err->printError();
        }
    } 
    else {
        se = getInfo().first;
        se->params.at(getInfo().second)->LLVMType = se->params.at(getInfo().second)->type->getLLVMType();
    }

    return nullptr;
}

/************************************/
/*             PAR GEN              */
/************************************/

llvm::Value* ParGen::compile() {
    par->setInfo(this->funcInfo);
    par->compile();
    if (parGen != nullptr) {
        parGen->setInfo(std::make_pair(this->funcInfo.first, this->funcInfo.second + 1));
        parGen->compile();
    }
    return nullptr;
}

/************************************/
/*                DEF               */
/************************************/

llvm::Value* Def::compile() {
    /* increase size of pseudoST for a new variable that was inserted */
    pseudoST.incrSize();
    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
    if(!mut) {
        /* if def is a function */
        if (parGen != nullptr) {
            int index = 0;
            ParGen *tempParGen = parGen;
            while (tempParGen != nullptr) {
                se->params.at(index++)->isVisible = true;
                /* increase size of pseudoST for a new function param that was inserted */
                pseudoST.incrSize();
                tempParGen = tempParGen->getNext();
            }
            currPseudoScope = currPseudoScope->getNext();
            currPseudoScope = currPseudoScope->getPrev();
        }
    }
    return nullptr;
}

/************************************/
/*             DEF GEN              */
/************************************/

llvm::Value* DefGen::compile() { return nullptr; }

/************************************/
/*                LET               */
/************************************/

llvm::Type *Let::getEnvStruct(SymbolEntry *se, std::vector<SymbolEntry *> &membersSE) {

    if (se->env == nullptr) {
        /* create custom struct needed for env */
        auto envStruct = llvm::StructType::get(TheContext);
        std::vector<llvm::Type *> members = {};
        if (rec) {
            members.push_back(se->type->getLLVMType());
            membersSE.push_back(se);
        }
        for (auto fv : freeVars) {
            if (!fv.compare(se->id) && rec) continue; // skip for name of rec function
            auto pse = currPseudoScope->lookup(fv, pseudoST.getSize());
            membersSE.push_back(pse);
            members.push_back(pse->type->getLLVMType());
        }

        envStruct->setBody(members);

        se->env = envStruct;
    }
    return se->env;
}

llvm::Value *Let::createTrampoline(SymbolEntry *se, std::vector<SymbolEntry *> membersSE) {

    auto envStruct = getEnvStruct(se, membersSE);

    /* create environment */
    auto envMallocSize = llvm::CallInst::CreateMalloc(
        Builder.GetInsertBlock(),
        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
        envStruct,
        llvm::ConstantExpr::getSizeOf(envStruct),
        nullptr,
        nullptr
    );
    auto envMalloc = Builder.Insert(envMallocSize, se->id + "_env");

    /* create trampoline */
    auto trampolineMallocSize = llvm::CallInst::CreateMalloc(
        Builder.GetInsertBlock(),
        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
        i8,
        llvm::ConstantExpr::getSizeOf(i8),
        c32(16),
        nullptr
    );
    auto trampolineMalloc = Builder.Insert(trampolineMallocSize, se->id + "_trampoline");

    /* fill env struct values */
    for (long unsigned int i = 0; i < membersSE.size(); i++) {
        /* access current position in env struct */
        auto currEnvPos = Builder.CreateGEP(envMalloc, {c32(0), c32(i)}, "currEnvPos_" + std::to_string(i));
        if (membersSE.at(i)->type->typeValue == TYPE_FUNC) membersSE.at(i)->functionEnvPtr = currEnvPos;
        else {
            /* get global llvm::Value* (freeVar) */
            auto currValue = Builder.CreateLoad(membersSE.at(i)->GlobalValue, membersSE.at(i)->id + "_globalVal");
            /* store loaded global value to current position in env struct */
            Builder.CreateStore(currValue, currEnvPos);
        }
    }

    /* initialize trampoline */
    llvm::Function *initTrampoline = llvm::Intrinsic::getDeclaration(TheModule.get(), llvm::Intrinsic::init_trampoline);
    Builder.CreateCall(
        initTrampoline,
        {
            trampolineMalloc,
            Builder.CreatePointerCast(se->Value, i8->getPointerTo()),
            Builder.CreatePointerCast(envMalloc, i8->getPointerTo())
        }
    );

    /* adjust trampoline */
    llvm::Function *adjustTrampoline = llvm::Intrinsic::getDeclaration(TheModule.get(), llvm::Intrinsic::adjust_trampoline);
    auto adjTrampoline = Builder.CreateCall(adjustTrampoline, { trampolineMalloc }, "adjTrampoline_" + se->id);

    return Builder.CreatePointerCast(adjTrampoline, se->LLVMType->getPointerTo(), "castTrampoline");

}

llvm::Value* Let::compile() {

    std::vector<SymbolEntry *> defsSE;
    std::vector<llvm::BasicBlock *> funcEntryBlocks;
    for (auto currDef : defs) currDef->compile();

    for (auto currDef : defs) {
        SymbolEntry *se = currPseudoScope->lookup(currDef->id, pseudoST.getSize());
        if (!currDef->mut && currDef->parGen == nullptr) se->isVisible = false;
        defsSE.push_back(se);
    }

    int seIndex = 0;
    for (auto currDef : defs) {

        SymbolEntry *se = defsSE.at(seIndex++);
        
        /* if def is a mutable variable/array */
        if (currDef->mut) {
            /* variable */
            if (currDef->expr == nullptr) {
                if (se != nullptr) {
                    auto mutableVarMalloc = llvm::CallInst::CreateMalloc(
                        Builder.GetInsertBlock(),
                        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                        se->type->getLLVMType()->getPointerElementType(),
                        llvm::ConstantExpr::getSizeOf(se->type->getLLVMType()->getPointerElementType()),
                        nullptr,
                        nullptr,
                        ""
                    );
                    se->Value = Builder.Insert(mutableVarMalloc, se->id);

                    if (se->isFreeVar) {
                        se->GlobalValue = createGlobalVariable(se->type->getLLVMType());
                        Builder.CreateStore(se->Value, se->GlobalValue);
                    }
                }
                else { std::cout << "Didn't find the se\n"; std::cout.flush(); }
            }
            /* array */
            else {
                if (se != nullptr) {
                    std::vector<llvm::Value *> dims;
                    dims.push_back(currDef->expr->compile());

                    /* size of array is at least one */
                    int dimNum = 1;

                    /* size of ith dimension saved in struct */
                    CommaExprGen *ceg = currDef->commaExprGen;
                    while (ceg != nullptr) {
                        dims.push_back(ceg->compile());
                        dimNum++;
                        ceg = ceg->getNext();
                    }

                    /* calculate total size of array */
                    llvm::Value *mulSize = dims.at(0);

                    for (long unsigned int i = 1; i < dims.size(); i++) {
                        mulSize = Builder.CreateMul(mulSize, dims.at(i));
                    }

                    se->LLVMType = se->type->getLLVMType()->getPointerElementType();

                    auto arrayMalloc = llvm::CallInst::CreateMalloc(
                        Builder.GetInsertBlock(),
                        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                        se->LLVMType,
                        llvm::ConstantExpr::getSizeOf(se->LLVMType),
                        nullptr,
                        nullptr,
                        ""
                    );
                    se->Value = Builder.Insert(arrayMalloc, se->id);
                    
                    auto arr = llvm::CallInst::CreateMalloc(
                        Builder.GetInsertBlock(),
                        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
                        se->type->ofType->getLLVMType(),
                        llvm::ConstantExpr::getSizeOf(se->type->ofType->getLLVMType()),
                        mulSize,
                        nullptr,
                        ""
                    );

                    Builder.Insert(arr, se->id);

                    /* append 'metadata' of the array variable { ptr_to_arr, dimsNum, dim1, dim2, ..., dimn } */
                    llvm::Value *arrayPtr = Builder.CreateGEP(se->LLVMType, se->Value, { c32(0), c32(0) }, "arrayPtr");
                    Builder.CreateStore(arr, arrayPtr);
                    llvm::Value *arrayDims = Builder.CreateGEP(se->LLVMType, se->Value, { c32(0), c32(1) }, "arrayDims");
                    Builder.CreateStore(c32(dimNum), arrayDims);
                    for (long unsigned int i = 0; i < dims.size(); i++) {
                        llvm::Value *dim = Builder.CreateGEP(se->LLVMType, se->Value, { c32(0), c32(i + 2) }, "dim_" + std::to_string(i));
                        Builder.CreateStore(dims.at(i), dim);
                    }

                    if (se->isFreeVar) {
                        se->GlobalValue  = createGlobalVariable(se->LLVMType->getPointerElementType());
                        Builder.CreateStore(se->Value, se->GlobalValue);    
                    }
                }
            }
        }
        else {
            se->isVisible = false;
            /* if def is a non mutable variable - constant */
            if (currDef->parGen == nullptr) {
                if (se != nullptr) {
                    se->Value = currDef->expr->compile();
                    if (se->isFreeVar) {
                        se->GlobalValue = createGlobalVariable(se->type->getLLVMType());
                        Builder.CreateStore(se->Value, se->GlobalValue);
                    }
                }
                /* left for debugging */
                else std::cout << "Symbol Entry was not found." << std::endl;
            }
            /* if def is a function */
            else {

                if (se->isFreeVar) se->GlobalValue = createGlobalVariable(se->type->getLLVMType());

                if (se != nullptr) {
                    if (rec) se->isVisible = true;
                    else se->isVisible = false;
                    std::vector<llvm::Type *> args;
                    currPseudoScope = currPseudoScope->getNext();

                    /* check below needed for closures */
                    long unsigned int parsGiven = 0;
                    ParGen *tempParGen = currDef->parGen;
                    while (tempParGen != nullptr) {
                        parsGiven++;
                        tempParGen = tempParGen->getNext();
                    }
                    if (parsGiven == se->params.size()) {
                        currDef->parGen->setInfo(std::make_pair(se, 0));
                        currDef->parGen->compile();
                        for (auto p : se->params) args.push_back(p->LLVMType);
                    }
                    else {
                        for (auto p : se->params) {
                            p->LLVMType = p->type->getLLVMType();
                            args.push_back(p->LLVMType);
                        }
                    }

                    llvm::Type *outputType = nullptr;
                    outputType = se->type->outputType->getLLVMType();
                    /* Initial function type without Nest - needs to be pure for the creation
                     of trampolines, therefore save it to se->LLVMType */
                    llvm::FunctionType *fType = llvm::FunctionType::get(outputType, args, false);
                    se->LLVMType = fType;


                    /* create env struct */
                    auto envStruct = getEnvStruct(se, currDef->freeVarsSE)->getPointerTo();
                    args.push_back(envStruct);
                    /* update fType to contain Nest to initialize function */
                    fType = llvm::FunctionType::get(outputType, args, false);

                    auto function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, se->id, TheModule.get());

                    unsigned index = 0;

                    for (auto &Arg : function->args()) {
                        if (index == se->params.size()) break;
                        Arg.setName(se->params.at(index++)->id);
                    }

                    index = 0;
                    for (auto &Arg : function->args()) {
                        if (index == se->params.size()) {
                            Arg.addAttr(llvm::Attribute::Nest);
                            currDef->nested = &Arg;
                            break;
                        }
                        se->params.at(index++)->Value = &Arg;
                    }

                    funcEntryBlocks.push_back(llvm::BasicBlock::Create(TheContext, "entry", function));

                    se->Value = function;

                    currPseudoScope = currPseudoScope->getPrev();
                }
                else std::cout << "Symbol Entry was not found." << std::endl;
            }
        }
    }

    for (auto se : defsSE) {
        se->isVisible = true;
        if (se->type->typeValue == TYPE_FUNC && !rec) se->isVisible = false;
    }

    for (auto currDef : defs) 
        if (!currDef->mut && currDef->parGen != nullptr) {
            currPseudoScope->currIndex--;
            if (defs.size() > 1) currPseudoScope->currIndex--;
        }

    int index = 0;
    for (auto currDef : defs) {
        if (!currDef->mut && currDef->parGen != nullptr) {
            SymbolEntry *se = defsSE.at(index);
            currPseudoScope = currPseudoScope->getNext();

            /* initialize trampoline */
            llvm::Value *newFunc = createTrampoline(se, currDef->freeVarsSE);
            newFunc->setName(se->Value->getName());
            se->Value = newFunc;
            if (se->isFreeVar) Builder.CreateStore(se->Value, se->GlobalValue);

            llvm::BasicBlock *Parent = Builder.GetInsertBlock();
            llvm::BasicBlock *FuncBB = funcEntryBlocks.at(index++);
            Builder.SetInsertPoint(FuncBB);

            for (auto p : se->params) {
                if (p->isFreeVar) {
                    if (p->GlobalValue == nullptr)p->GlobalValue = createGlobalVariable(p->LLVMType);
                    Builder.CreateStore(p->Value, p->GlobalValue);
                }
            }

            long unsigned int freeSEIndex = 0;
            std::vector<llvm::Value *> prevVals;
            for (auto freeSE : currDef->freeVarsSE) {
                prevVals.push_back(freeSE->Value);
                freeSE->Value = Builder.CreateLoad(Builder.CreateGEP(currDef->nested, { c32(0), c32(freeSEIndex++) }));
            }

            llvm::Value *returnExpr = currDef->expr->compile();

            if (!se->params.at(0)->id.compare(se->id + "_param_0")) {
                std::vector<llvm::Value *> args;
                llvm::Value *v;
                for (long unsigned int i = 0; i < se->params.size(); i++) {
                    v = se->params.at(i)->Value;
                    args.push_back(v);
                }
                Builder.CreateRet(Builder.CreateCall(returnExpr, args));
            }
            else Builder.CreateRet(returnExpr);
            
            Builder.SetInsertPoint(Parent);

            int index = 0;
            for (auto freeSE : currDef->freeVarsSE) {
                if (freeSE->type->typeValue != TYPE_FUNC) freeSE->Value = prevVals.at(index);
                else {
                    freeSE->Value = Builder.CreateLoad(freeSE->GlobalValue);
                    if (freeSE->functionEnvPtr != nullptr) Builder.CreateStore(freeSE->Value, freeSE->functionEnvPtr);
                }
                index++;
            }

            currPseudoScope = currPseudoScope->getPrev();

        }
    }
    for (auto se : defsSE) se->isVisible = true;

    if(defs.size() > 1) {
        for (auto currDef : defs) {
            if (!currDef->mut && currDef->parGen != nullptr) {
                currPseudoScope->currIndex++;
            }
        }
    }

    return nullptr;

}

/************************************/
/*              LETIN               */
/************************************/

llvm::Value* LetIn::compile() {
    currPseudoScope = currPseudoScope->getNext();
    let->compile();
    llvm::Value *rv = expr->compile();
    currPseudoScope = currPseudoScope->getPrev();
    return rv;
}

/************************************/
/*              DELETE              */
/************************************/

llvm::Value* Delete::compile() {
    Builder.Insert(llvm::CallInst::CreateFree(expr->compile(), Builder.GetInsertBlock()));
    return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
}

/************************************/
/*                NEW               */
/************************************/

llvm::Value* New::compile() {
    auto v = llvm::CallInst::CreateMalloc(
        Builder.GetInsertBlock(),
        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
        type->getLLVMType()->getPointerElementType(),
        llvm::ConstantExpr::getSizeOf(type->getLLVMType()->getPointerElementType()),
        nullptr,
        nullptr,
        ""
    );

    return Builder.Insert(v);
}

/************************************/
/*             ARRAYITEM            */
/************************************/

llvm::Value* ArrayItem::compile() {
    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
    if (se != nullptr) {
        llvm::Value *accessEl = nullptr;
        std::vector<llvm::Value *> dims;
        llvm::Value *mulTemp = c32(1);

        dims.push_back(expr->compile());
        CommaExprGen *ceg = commaExprGen;
        while (ceg != nullptr) {
            dims.push_back(ceg->compile());
            ceg = ceg->getNext();
        }

        llvm::Value *accessSeValue = se->Value;

        for (long unsigned int i = dims.size(); i > 0; i--) {
            if (i != dims.size()) {
                mulTemp = Builder.CreateMul(
                    mulTemp,
                    Builder.CreateLoad(
                        Builder.CreateGEP(
                            (se->LLVMType->isPointerTy()) ? se->LLVMType->getPointerElementType() : se->LLVMType,
                            accessSeValue,
                            std::vector<llvm::Value *> {c32(0), c32(i + 2)}
                        )
                    ));
                accessEl = Builder.CreateAdd(accessEl, Builder.CreateMul(mulTemp, dims.at(i - 1)));
            }
            else {
                accessEl = dims.at(i - 1);
            }
        }
        
        /* check access_dim.at(i) with decl_dim.at(i), if all acccess_dims are less than decl_dims all good else problem */
        llvm::Value *isCorrect = c1(true);
        llvm::Value *isGT;
        for (long unsigned int i = 0; i < dims.size(); i++) {
            isGT = Builder.CreateICmpSLT(dims.at(i), Builder.CreateLoad(Builder.CreateGEP((se->LLVMType->isPointerTy()) ? se->LLVMType->getPointerElementType() : se->LLVMType, accessSeValue, {c32(0), c32(i + 2)})));
            isCorrect = Builder.CreateAnd(isGT, isCorrect);
        }

        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
        
        /* create ir for branch */
        llvm::BasicBlock *RuntimeExceptionBB = llvm::BasicBlock::Create(TheContext);
        llvm::BasicBlock *ContinueBB = llvm::BasicBlock::Create(TheContext);
        Builder.CreateCondBr(isCorrect, ContinueBB, RuntimeExceptionBB);     

        /* in case that access element is out of bounds */
        TheFunction->getBasicBlockList().push_back(RuntimeExceptionBB);
        Builder.SetInsertPoint(RuntimeExceptionBB);
        Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Index out of Bounds\n")) });
        Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
        Builder.CreateBr(ContinueBB);

        /* in case that all good */
        TheFunction->getBasicBlockList().push_back(ContinueBB);
        Builder.SetInsertPoint(ContinueBB);
        llvm::Value *arrPtr = Builder.CreateGEP((se->LLVMType->isPointerTy()) ? se->LLVMType->getPointerElementType() : se->LLVMType, accessSeValue, std::vector<llvm::Value *> {c32(0), c32(0)});
        arrPtr = Builder.CreateLoad(arrPtr, se->id);
        return Builder.CreateGEP(arrPtr, accessEl);
    }

    return nullptr;

}

/************************************/
/*                DIM               */
/************************************/

llvm::Value* Dim::compile() {
    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
    if (se != nullptr) {
        llvm::Value *accessSeValue = se->Value;
        return Builder.CreateLoad(Builder.CreateGEP(accessSeValue, { c32(0), c32(intconst + 1) }), se->id + "_dim");
    }

    return nullptr;
}

/************************************/
/*               BINOP              */
/************************************/

llvm::Value* BinOp::generalTypeCheck(llvm::Value *val1, llvm::Value *val2, CustomType* ct) {
    if (ct->typeValue == TYPE_UNIT) return c1(true);
    if (ct->typeValue == TYPE_ID || ct->typeValue == TYPE_CUSTOM) {
        llvm::BasicBlock *parentBB =  Builder.GetInsertBlock();
        llvm::Function *cstTypeEqFunc = constrsEqCheck(val1, val2, currPseudoScope->lookup(ct->getName(), pseudoST.getSize()));
        Builder.SetInsertPoint(parentBB);
        return Builder.CreateCall(cstTypeEqFunc, {val1, val2});
    }
    if (ct->typeValue == TYPE_INT || ct->typeValue == TYPE_CHAR || ct->typeValue == TYPE_BOOL) {
        return Builder.CreateICmpEQ(val1, val2);
    }
    if (ct->typeValue == TYPE_FLOAT) {
        return Builder.CreateFCmpOEQ(val1, val2);
    }
    if (ct->typeValue == TYPE_FUNC) {
        Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Cannot compare functional values.\n")) });
        Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
        return c1(true);
    }
    return nullptr;
}

llvm::Function* BinOp::constrsEqCheck(llvm::Value *constr1, llvm::Value *constr2, SymbolEntry *baseTypeSE) {
    /* if function to compare current type exists, return it */
    if(TheModule->getFunction("constrEqCheck_" + baseTypeSE->id)) return TheModule->getFunction("constrEqCheck_" + baseTypeSE->id);

    /* create function */
    llvm::FunctionType *struct_type = llvm::FunctionType::get(i1, std::vector<llvm::Type *> { constr1->getType(), constr2->getType() }, false);
    auto *TheStructCmp = llvm::Function::Create(struct_type, llvm::Function::ExternalLinkage, "constrEqCheck_" + baseTypeSE->id, TheModule.get());

    /* create necessary blocks */
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(TheContext, "entry", TheStructCmp);
    llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(TheContext, "exit", TheStructCmp);
    llvm::BasicBlock *switchBlock = llvm::BasicBlock::Create(TheContext, "switch", TheStructCmp);
    llvm::BasicBlock *errorBlock = llvm::BasicBlock::Create(TheContext, "error", TheStructCmp);

    Builder.SetInsertPoint(exitBlock);
    /* create phi node for switch with as many incoming blocks as constructors in type + 1 for default */
    int incomingBlocks = 0;
    for (auto constrSE : baseTypeSE->params) 
        incomingBlocks += (constrSE->type->params.size()) ? constrSE->type->params.size() : 1;
    llvm::PHINode *resPhi = Builder.CreatePHI(i1, incomingBlocks+1);

    Builder.SetInsertPoint(entryBlock);
    /* compare tags */
    llvm::Value *lhsTag, *rhsTag, *compRes;
    lhsTag = Builder.CreateLoad(Builder.CreateGEP(TheStructCmp->getArg(0), {c32(0), c32(0)}, "lhsTag"));
    rhsTag = Builder.CreateLoad(Builder.CreateGEP(TheStructCmp->getArg(1), {c32(0), c32(0)}, "rhsTag"));
    compRes = Builder.CreateICmpEQ(lhsTag, rhsTag, "isEq");

    /* in case tags are not the same */
    Builder.CreateCondBr(compRes, switchBlock, exitBlock);
    resPhi->addIncoming(compRes, entryBlock);

    Builder.SetInsertPoint(switchBlock);
    /* create switch blocks */
    std::vector<llvm::BasicBlock *> switchBlocks;
    llvm::SwitchInst *typeSwitch = Builder.CreateSwitch(lhsTag, errorBlock, baseTypeSE->params.size());
    llvm::BasicBlock *currentBlock;
    /* create one case for every switch statement */
    for (long unsigned int i = 0; i < baseTypeSE->params.size(); i++) {
        currentBlock = llvm::BasicBlock::Create(TheContext, std::string("case_") + baseTypeSE->params.at(i)->id, TheStructCmp);
        switchBlocks.push_back(currentBlock);
        typeSwitch->addCase(c32(i), currentBlock);
    }

    /* switch default case is error*/
    Builder.SetInsertPoint(errorBlock);
    Builder.CreateCall(TheModule->getFunction("writeString"), {Builder.CreateGlobalStringPtr("Internal error: Invalid constructor enum\n")});
    Builder.CreateCall(TheModule->getFunction("exit"), {c32(1)});
    Builder.CreateBr(errorBlock);

    /* for every constructor check inner fields */
    for (long unsigned int i = 0; i < baseTypeSE->params.size(); i++) {
        llvm::Value *lhsCastedVal, *rhsCastedVal;
        llvm::StructType *currConstrType = TheModule->getTypeByName(baseTypeSE->id + "_" + baseTypeSE->params.at(i)->id);
        /* write on current switch block*/
        currentBlock = switchBlocks[i];
        Builder.SetInsertPoint(currentBlock);
        
        /* no fields in current constructor, therefore no checks are needed */
        if (dynamic_cast<CustomId *>(baseTypeSE->params.at(i)->type)->getParams().size() == 0) {
            Builder.CreateBr(exitBlock);
            resPhi->addIncoming(c1(true), currentBlock);
            continue;
        }

        /* get IR type for current constructor */
        lhsCastedVal = Builder.CreatePointerCast(TheStructCmp->getArg(0), currConstrType->getPointerTo(), "lhsCast");
        rhsCastedVal = Builder.CreatePointerCast(TheStructCmp->getArg(1), currConstrType->getPointerTo(), "rhsCast");
        /* check all fields of current constructor */
        for (long unsigned int j = 0; j < dynamic_cast<CustomId *>(baseTypeSE->params.at(i)->type)->getParams().size(); j++) {
            /* get field j for both constructors */                
            llvm::Value *lhsField = Builder.CreateLoad(Builder.CreateGEP(lhsCastedVal, {c32(0), c32(j+1)}));
            llvm::Value *rhsField = Builder.CreateLoad(Builder.CreateGEP(rhsCastedVal, {c32(0), c32(j+1)}));
            /* compare the two fields */
            compRes = generalTypeCheck(lhsField, rhsField, dynamic_cast<CustomId *>(baseTypeSE->params.at(i)->type)->getParams().at(j));

            /* create blocks to branch for each field */
            llvm::BasicBlock *nextFieldBB = llvm::BasicBlock::Create(TheContext, std::string("case_") + baseTypeSE->params.at(i)->id + "_nextfield", TheStructCmp);
            Builder.CreateCondBr(compRes, nextFieldBB, exitBlock);
            resPhi->addIncoming(compRes, Builder.GetInsertBlock());
            Builder.SetInsertPoint(nextFieldBB);
        }
        /* branch to exit block */
        Builder.CreateBr(exitBlock);
        resPhi->addIncoming(compRes, Builder.GetInsertBlock());
    }
    Builder.SetInsertPoint(exitBlock);
    Builder.CreateRet(resPhi);

    TheFPM->run(*TheStructCmp);
    return TheStructCmp;
}

llvm::Value* BinOp::compile() {
    llvm::Value *lv = expr1->compile();
    llvm::Value *rv = expr2->compile();

    if (lv != nullptr && lv->getType()->isPointerTy() && strcmp(op, ":=") && getRefFinalType(expr1->getType()).first->typeValue != TYPE_CUSTOM) lv = Builder.CreateLoad(lv);
    // if (rv != nullptr && rv->getType()->isPointerTy() && strcmp(op, ";") && getRefFinalType(expr2->getType()).first->typeValue != TYPE_CUSTOM && getRefFinalType(expr2->getType()).first->typeValue != TYPE_FUNC) rv = Builder.CreateLoad(rv);

    if (!strcmp(op, "+")) return Builder.CreateAdd(lv, rv);
    else if (!strcmp(op, "-")) return Builder.CreateSub(lv, rv);
    else if (!strcmp(op, "*")) return Builder.CreateMul(lv, rv);
    else if (!strcmp(op, "/")) {
        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
        
        /* check if rhs is eq to zero */
        llvm::Value *isZero = Builder.CreateICmpEQ(rv, c32(0));

        /* create ir for branch */
        llvm::BasicBlock *RuntimeExceptionBB = llvm::BasicBlock::Create(TheContext);
        llvm::BasicBlock *ContinueBB = llvm::BasicBlock::Create(TheContext);
        Builder.CreateCondBr(isZero, RuntimeExceptionBB, ContinueBB);

        /* in case that it is, then Runtime Exception should be raised */
        TheFunction->getBasicBlockList().push_back(RuntimeExceptionBB);
        Builder.SetInsertPoint(RuntimeExceptionBB);
        Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Division with Zero\n")) });
        Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
        Builder.CreateBr(ContinueBB);

        /* else should continue normally */
        TheFunction->getBasicBlockList().push_back(ContinueBB);
        Builder.SetInsertPoint(ContinueBB);
        return Builder.CreateSDiv(lv, rv);
    }
    else if (!strcmp(op, "mod")) return Builder.CreateSRem(lv, rv);
    else if (!strcmp(op, "+.")) return Builder.CreateFAdd(lv, rv);
    else if (!strcmp(op, "-.")) return Builder.CreateFSub(lv, rv);
    else if (!strcmp(op, "*.")) return Builder.CreateFMul(lv, rv);
    else if (!strcmp(op, "/.")) {
        llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

        /* check if rhs is eq to zero */
        llvm::Value *isZero = Builder.CreateFCmpOEQ(rv, fp(0));

        /* create ir for branch */
        llvm::BasicBlock *RuntimeExceptionBB = llvm::BasicBlock::Create(TheContext);
        llvm::BasicBlock *ContinueBB = llvm::BasicBlock::Create(TheContext);
        Builder.CreateCondBr(isZero, RuntimeExceptionBB, ContinueBB);

        /* in case that it is, then Runtime Exception should be raised */
        TheFunction->getBasicBlockList().push_back(RuntimeExceptionBB);
        Builder.SetInsertPoint(RuntimeExceptionBB);
        Builder.CreateCall(TheModule->getFunction("writeString"), { Builder.CreateGlobalStringPtr(llvm::StringRef("Runtime Error: Division with Zero\n")) });
        Builder.CreateCall(TheModule->getFunction("exit"), { c32(1) });
        Builder.CreateBr(ContinueBB);

        /* else should continue normally */
        TheFunction->getBasicBlockList().push_back(ContinueBB);
        Builder.SetInsertPoint(ContinueBB);
        return Builder.CreateFDiv(lv, rv);
    }
    else if (!strcmp(op, "**")) return Builder.CreateBinaryIntrinsic(llvm::Intrinsic::pow, lv, rv, nullptr, "float.powtmp");
    else if (!strcmp(op, "=")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_CUSTOM:
                return generalTypeCheck(lv, rv, getRefFinalType(expr1->getType()).first);
            case TYPE_UNIT:
                return c1(true);
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_OEQ, lv, rv);
            case TYPE_BOOL:
                return Builder.CreateNot(Builder.CreateOr(lv, rv));
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_EQ, lv, rv);
        }
    }
    else if (!strcmp(op, "<>")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_CUSTOM:
                return Builder.CreateNot(generalTypeCheck(lv, rv, getRefFinalType(expr1->getType()).first));
            case TYPE_UNIT:
                return c1(false);
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_ONE, lv, rv);
            case TYPE_BOOL:
                return Builder.CreateOr(lv, rv);
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_NE, lv, rv);
        }
    }
    else if (!strcmp(op, "==")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_CUSTOM:
                return Builder.CreateICmpEQ(Builder.CreatePtrDiff(lv, rv), c64(0));
            case TYPE_UNIT:
                return c1(true);
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_OEQ, lv, rv);
            case TYPE_BOOL:
                return Builder.CreateNot(Builder.CreateOr(lv, rv));
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_EQ, lv, rv);
        }
    }
    else if (!strcmp(op, "!=")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_CUSTOM:
                return Builder.CreateICmpNE(Builder.CreatePtrDiff(lv, rv), c64(0));
            case TYPE_UNIT:
                return c1(false);
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_ONE, lv, rv);
            case TYPE_BOOL:
                return Builder.CreateOr(lv, rv);
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_NE, lv, rv);
        }
    }
    else if (!strcmp(op, "<")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_OLT, lv, rv);
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_SLT, lv, rv);
        }
    }
    else if (!strcmp(op, ">")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_OGT, lv, rv);
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_SGT, lv, rv);
        }
    }
    else if (!strcmp(op, ">=")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_OGE, lv, rv);
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_SGE, lv, rv);
        }
    }
    else if (!strcmp(op, "<=")) {
        switch (getRefFinalType(expr1->getType()).first->typeValue) {
            case TYPE_FLOAT:
                return Builder.CreateFCmp(llvm::CmpInst::FCMP_OLE, lv, rv);
            case TYPE_CHAR:
            default:
                return Builder.CreateICmp(llvm::CmpInst::ICMP_SLE, lv, rv);
        }
    }
    else if (!strcmp(op, "&&")) return Builder.CreateAnd(lv, rv);
    else if (!strcmp(op, "||")) return Builder.CreateOr(lv, rv);
    else if (!strcmp(op, ";")) return rv;
    else if (!strcmp(op, ":=")) {
        Builder.CreateStore(rv, lv);
        return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));
    }

    return nullptr;
}

/************************************/
/*                UNOP              */
/************************************/

llvm::Value* UnOp::compile() {
    llvm::Value *v = expr->compile();
    if (!strcmp(op, "!")) {
        if (v->getType()->isPointerTy()) return Builder.CreateLoad(v);
        return v;
    }
    else if (!strcmp(op, "+")) return v;
    else if (!strcmp(op, "-")) {
        return Builder.CreateMul(v, c32(-1));
    }
    else if (!strcmp(op, "+.")) return v;
    else if (!strcmp(op, "-.")) {
        return Builder.CreateFMul(v, fp(-1.0));
    }
    else if (!strcmp(op, "not")) return Builder.CreateNot(v);
    return nullptr;
}

/************************************/
/*             INTCONST             */
/************************************/

llvm::Value* IntConst::compile() {
    if (isPattern) return Builder.CreateICmpEQ(c32(intConst), matchExprV);
    else return c32(intConst);
}

/************************************/
/*            FLOATCONST            */
/************************************/

llvm::Value* FloatConst::compile() {
    if (isPattern) return Builder.CreateFCmpOEQ(fp(floatConst), matchExprV);
    else return fp(floatConst);
}

/************************************/
/*             CHARCONST            */
/************************************/

llvm::Value* CharConst::compile() {
    if (isPattern) return Builder.CreateICmpEQ(c8(charConst), matchExprV);
    else return c8(charConst);
}

/************************************/
/*          STRINGLITERAL           */
/************************************/

llvm::Value* StringLiteral::compile() {

    llvm::StructType *arrayStruct = TheModule->getTypeByName("Array_Character_1");

    auto stringVarMalloc = llvm::CallInst::CreateMalloc(
        Builder.GetInsertBlock(),
        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
        arrayStruct,
        llvm::ConstantExpr::getSizeOf(arrayStruct),
        nullptr,
        nullptr,
        ""
    );
    llvm::Value *stringV = Builder.Insert(stringVarMalloc);


    auto arr = llvm::CallInst::CreateMalloc(
        Builder.GetInsertBlock(),
        llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
        i8,
        llvm::ConstantExpr::getSizeOf(i8),
        c32(stringLiteral.length()),
        nullptr,
        ""
    );

    Builder.Insert(arr);

    /* append 'metadata' of the array variable { ptr_to_arr, dimsNum, dim1, dim2, ..., dimn } */
    llvm::Value *arrayPtr = Builder.CreateGEP(arrayStruct, stringV, { c32(0), c32(0) }, "stringLiteral");
    Builder.CreateStore(arr, arrayPtr);
    llvm::Value *arrayDims = Builder.CreateGEP(arrayStruct, stringV, { c32(0), c32(1) }, "stringDim");
    Builder.CreateStore(c32(1), arrayDims);
    llvm::Value *dim = Builder.CreateGEP(arrayStruct, stringV, { c32(0), c32(2) }, "dim_0");
    Builder.CreateStore(c32(stringLiteral.length()), dim);

    /* add the string to the array */
    std::vector<llvm::Value *> args;
    args.push_back(Builder.CreateLoad(arrayPtr));
    args.push_back(Builder.CreateGlobalStringPtr(llvm::StringRef(stringLiteral)));
    Builder.CreateCall(TheStringCopy, args);

    return stringV;
}

/************************************/
/*           BOOLEANCONST           */
/************************************/

llvm::Value* BooleanConst::compile() {
    if (isPattern) return Builder.CreateAnd(c1(boolean), matchExprV);
    else return c1(boolean);
}

/************************************/
/*            UNITCONST             */
/************************************/

llvm::Value* UnitConst::compile() { return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit")); }

/************************************/
/*             TYPEGEN              */
/************************************/

llvm::Value* TypeGen::compile() { return nullptr; }

/************************************/
/*              CONSTR              */
/************************************/

void Constr::defineConstr(SymbolEntry *se) {
    std::string constrName = se->params.front()->id + "_" + se->id;

    /* create constr */
    std::vector<llvm::Type *> members;

    /* tag */
    members.push_back(i32);

    /* append all necessary fields in constructor Struct */
    for (auto p : dynamic_cast<CustomId *>(se->type)->getParams()) {
        members.push_back(p->getLLVMType());
    }

    /* create the constr */
    llvm::StructType *constrStruct = llvm::StructType::create(TheContext, constrName);
    constrStruct->setBody(members);

    se->LLVMType = constrStruct;
}

llvm::Value* Constr::compile() {

    SymbolEntry *se = currPseudoScope->lookup(Id, pseudoST.getSize());
    if (!call) {
        if (se != nullptr) defineConstr(se);
    }
    else {
        auto structMalloc = llvm::CallInst::CreateMalloc(
            Builder.GetInsertBlock(),
            llvm::Type::getIntNTy(TheContext, TheModule->getDataLayout().getMaxPointerSizeInBits()),
            se->LLVMType,
            llvm::ConstantExpr::getSizeOf(se->LLVMType),
            nullptr,
            nullptr,
            ""
        );

        llvm::Value *v = Builder.Insert(structMalloc, Id);

        llvm::Value *tag = Builder.CreateGEP(se->LLVMType, v, { c32(0), c32(0) }, "tag");
        std::vector<SymbolEntry *> udtSE = se->params.front()->params;
        int index = 0;
        for (long unsigned int i = 0; i < udtSE.size(); i++) {
            if (se == udtSE.at(i)) index = i;
        }
        Builder.CreateStore(c32(index), tag);
        
        if (expr != nullptr) {
            llvm::Value *temp = Builder.CreateGEP(se->LLVMType, v, { c32(0), c32(1) }, "temp");
            Builder.CreateStore(expr->compile(), temp);
        }
        if (exprGen != nullptr) {
            index = 2;
            ExprGen *tempExprGen = exprGen;
            llvm::Value *temp;
            while (tempExprGen != nullptr) {
                temp = Builder.CreateGEP(se->LLVMType, v, { c32(0), c32(index++) }, "temp");
                Builder.CreateStore(tempExprGen->compile(), temp);
                tempExprGen = tempExprGen->getNext();
            }
        }

        /* in case the expr is a constructor need bitcast to convert v to base type */
        SymbolEntry *exprSE = currPseudoScope->lookup(Id, pseudoST.getSize());
        return Builder.CreatePointerCast(structMalloc, exprSE->params.front()->LLVMType->getPointerTo());
        
    }

    return nullptr;
}

/************************************/
/*            BARCONSTRGEN          */
/************************************/

llvm::Value* BarConstrGen::compile() {
    constr->compile();
    if (barConstrGen != nullptr) barConstrGen->compile();

    return nullptr;
}

/************************************/
/*                TDEF              */
/************************************/

llvm::Value* Tdef::compile() {

    SymbolEntry *se = currPseudoScope->lookup(id, pseudoST.getSize());
    if (se != nullptr) {
        constr->compile();
        if (barConstrGen != nullptr) barConstrGen->compile();
    }

    return nullptr;
}

/************************************/
/*              TDEFGEN             */
/************************************/

llvm::Value* TdefGen::compile() {
    tDef->compile();
    if (tDefGen != nullptr) tDefGen->compile();

    return nullptr;
}

/************************************/
/*              TYPEDEF             */
/************************************/

void TypeDef::defineUDT(Tdef *td) {
    pseudoST.incrSize();
    SymbolEntry *tdSE = currPseudoScope->lookup(td->getName(), pseudoST.getSize());
    if (tdSE != nullptr) {

        std::string udtName = tdSE->id;
        
        /* create udt */
        std::vector<llvm::Type *> members;
        /* tag */
        members.push_back(i32);

        /* create the udt */
        llvm::StructType *udtStruct = llvm::StructType::create(TheContext, udtName);
        udtStruct->setBody(members);

        tdSE->LLVMType = udtStruct;

        /* increment number of variables (Constr) */
        pseudoST.incrSize();

        /* increment number of variables (BarConstrGen) */
        BarConstrGen *bcg = td->getBarConstrGen();
        while (bcg != nullptr) {
            pseudoST.incrSize();
            bcg = bcg->getNext();
        }
    }
}

llvm::Value* TypeDef::compile() {
        
    /* define first udt */
    defineUDT(tDef);

    /* declare all udt type a = ... and b = ... */
    if (tDefGen != nullptr) {
        TdefGen *tdg = tDefGen;
        while (tdg != nullptr) {
            defineUDT(tdg->getTdef());
            tdg = tdg->getNext();
        }
    }

    tDef->compile();
    if (tDefGen != nullptr) tDefGen->compile();

    return nullptr;
}

/************************************/
/*       TYPES.HPP - CUSTOMTYPE     */
/************************************/

llvm::Value* CustomType::compile() {
    return 0;
}

llvm::Type* CustomType::getLLVMType() {
    if (typeValue == TYPE_INT) return i32;
    if (typeValue == TYPE_FLOAT) return DoubleTyID;
    if (typeValue == TYPE_BOOL) return i1;
    if (typeValue == TYPE_CHAR) return i8;
    if (typeValue == TYPE_UNIT) return TheModule->getTypeByName("unit");
    if (typeValue == TYPE_REF) return ofType->getLLVMType()->getPointerTo();
    if (typeValue == TYPE_UNKNOWN) return TheModule->getTypeByName("unit"); 
    if (typeValue == TYPE_CUSTOM) return currPseudoScope->lookupTypes(name, pseudoST.getSize())->LLVMType->getPointerTo();
    if (typeValue == TYPE_ARRAY) {
        /* name of struct type that we're searching */
        std::string arrName = "Array_" + ofType->getName() + "_" + std::to_string(size);

        if (TheModule->getTypeByName(arrName) != nullptr) {
            return TheModule->getTypeByName(arrName)->getPointerTo();
        }

        /* create array */
        std::vector<llvm::Type *> members;
        /* ptr to array */
        members.push_back(llvm::PointerType::getUnqual(ofType->getLLVMType()));
        /* dimensions number of array */
        members.push_back(i32);

        for (int i = 0; i < size; i++) members.push_back(i32);         

        /* create the struct */
        llvm::StructType *arrayStruct = llvm::StructType::create(TheContext, arrName);
        arrayStruct->setBody(members);

        return arrayStruct->getPointerTo();
    }
    if (typeValue == TYPE_FUNC) {
        std::vector<llvm::Type *> args;

        for (auto p : params) args.push_back(p->getLLVMType());    
        
        return llvm::FunctionType::get(outputType->getLLVMType(), args, false)->getPointerTo();
    }

    return nullptr;
}

/************************************/
/*         TYPES.HPP - UNIT         */
/************************************/

llvm::Value *Unit::compile() { return 0; }

/************************************/
/*        TYPES.HPP - INTEGER       */
/************************************/

llvm::Value* Integer::compile() { return 0; }

/************************************/
/*        TYPES.HPP - CHARACTER     */
/************************************/

llvm::Value* Character::compile() { return 0; }

/************************************/
/*        TYPES.HPP - BOOLEAN       */
/************************************/

llvm::Value* Boolean::compile() { return 0; }

/************************************/
/*          TYPES.HPP - FLOAT       */
/************************************/

llvm::Value* Float::compile() { return 0; }

/************************************/
/*        TYPES.HPP - FUNCTION      */
/************************************/

llvm::Value* Function::compile() { return 0; }

/************************************/
/*        TYPES.HPP - REFERENCE     */
/************************************/

llvm::Value* Reference::compile() { return 0; }

/************************************/
/*         TYPES.HPP - ARRAY        */
/************************************/

llvm::Value* Array::compile() { return 0; }

/************************************/
/*         TYPES.HPP - CUSTOMID     */
/************************************/

llvm::Value* CustomId::compile() { return 0; }
/************************************/
/*        TYPES.HPP - UNKNOWN       */
/************************************/

llvm::Value* Unknown::compile() { return 0; }