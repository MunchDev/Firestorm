//
// Created by Nguyen Thai Binh on 18/1/22.
//
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/InstSimplifyPass.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>

#include "codegen.hpp"

namespace Firestorm::AST {

    CodeGenerator::CodeGenerator() : context(), builder(context), module("Main", context),
                                     optimiser(module) {}

    Optimiser::Optimiser(llvm::Module &m) {
        // Do simple "peephole" optimizations and bit-twiddling options.
        passManager.addPass(llvm::InstCombinePass());
        // Re-associate expressions.
        passManager.addPass(llvm::ReassociatePass());
        // Eliminate Common SubExpressions.
        passManager.addPass(llvm::GVN());
        // Simplify the control flow graph (deleting unreachable blocks, etc).
        passManager.addPass(llvm::SimplifyCFGPass());
    }
}
