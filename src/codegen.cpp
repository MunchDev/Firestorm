//
// Created by Nguyen Thai Binh on 18/1/22.
//
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include "codegen.hpp"

CodeGenerator::CodeGenerator() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("Main", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    passManager = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());

    // Do simple "peephole" optimizations and bit-twiddling options.
    passManager->add(llvm::createInstructionCombiningPass());
    // Re-associate expressions.
    passManager->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    passManager->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    passManager->add(llvm::createCFGSimplificationPass());

    passManager->doInitialization();
}
