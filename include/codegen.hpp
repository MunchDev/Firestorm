//
// Created by Nguyen Thai Binh on 18/1/22.
//

#ifndef FIRESTORM_CODEGEN_HPP
#define FIRESTORM_CODEGEN_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Module.h>

namespace Firestorm::AST {
    /// @brief Contains LLVM's optimisation passes to run when compiling Firestorm code.
    struct Optimiser {
        llvm::FunctionPassManager passManager;

        explicit Optimiser(llvm::Module& m);
    };

    /// @brief Contains LLVM elements used to emit LLVM IR for Firestorm code.
    struct CodeGenerator {
        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        llvm::Module module;
        Optimiser optimiser;
        std::map<std::string, llvm::Value *> namedValues;

        CodeGenerator();

        CodeGenerator(const CodeGenerator &) = delete;

        CodeGenerator(CodeGenerator &&) = delete;

        void operator=(const CodeGenerator &) = delete;

        void operator=(CodeGenerator &&) = delete;
    };
}

#endif //FIRESTORM_CODEGEN_HPP
