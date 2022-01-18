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
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>

#include "utility.hpp"

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct CodeGenerator {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::legacy::FunctionPassManager> passManager;
    std::map<std::string, llvm::Value *> namedValues;


    CodeGenerator();

    CodeGenerator(const CodeGenerator &) = delete;

    CodeGenerator(CodeGenerator &&) = delete;

    void operator=(const CodeGenerator &) = delete;

    void operator=(CodeGenerator &&) = delete;
};

class CodegenError : public FirestormError {
public:
    explicit CodegenError(const char *msg) : FirestormError(msg) {}
};

#endif //FIRESTORM_CODEGEN_HPP
