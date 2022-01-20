//
// Created by Nguyen Thai Binh on 18/1/22.
//
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>

#include "codegen.hpp"

namespace Firestorm::AST {

    CodeGenerator::CodeGenerator() : context(), builder(context), module("Main", context),
                                     optimiser(module) {}

    Optimiser::Optimiser(llvm::Module &m) : passManager(&m) {
        // Do simple "peephole" optimizations and bit-twiddling options.
        passManager.add(llvm::createInstructionCombiningPass());

        // Re-associate expressions.
        passManager.add(llvm::createReassociatePass());

        // Eliminate Common SubExpressions.
        passManager.add(llvm::createGVNPass());

        // Simplify the control flow graph (deleting unreachable blocks, etc).
        passManager.add(llvm::createCFGSimplificationPass());

        passManager.doInitialization();
    }
}
