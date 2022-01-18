//
// Created by Nguyen Thai Binh on 17/1/22.
//
#include <sstream>
#include <fmt/format.h>

#include "ast.hpp"
#include "codegen.hpp"

using fmt::format;

CodeGenerator& getCodegen() {
    static CodeGenerator codegen;
    return codegen;
}

auto& Context() {
    return getCodegen().context;
}

auto& Builder() {
    return getCodegen().builder;
}

auto& Module() {
    return getCodegen().module;
}

auto& PassManager() {
    return getCodegen().passManager;
}

auto& NamedValues() {
    return getCodegen().namedValues;
}

std::string NumberExpr::toString() const {
    return format("Number({})", value);
}

llvm::Value *NumberExpr::generateIR() const {
    return llvm::ConstantFP::get(*Context(), llvm::APFloat(value));
}

std::string VariableExpr::toString() const {
    return format("Variable({})", name);
}

llvm::Value *VariableExpr::generateIR() const {
    // Look up if variable declared
    auto value = NamedValues()[name];
    if (!value) {
        throw CodegenError(fmt::format("Unknown variable '{}'", name).c_str());
    }
    return value;
}

std::string BinaryExpr::toString() const {
    auto l = lhs->toString();
    auto r = rhs->toString();
    return format("BinOp(lhs={}, op='{}', rhs={})", l, op, r);
}

llvm::Value *BinaryExpr::generateIR() const {
    auto lhs_code = lhs->generateIR();
    auto rhs_code = rhs->generateIR();

    if (op == "+")
        return Builder()->CreateFAdd(lhs_code, rhs_code, "add_tmp");
    else if (op == "-")
        return Builder()->CreateFSub(lhs_code, rhs_code, "sub_tmp");
    else if (op == "*")
        return Builder()->CreateFMul(lhs_code, rhs_code, "mul_tmp");
    else if (op == "/")
        return Builder()->CreateFDiv(lhs_code, rhs_code, "div_tmp");
    else if (op == "==") {
        lhs_code = Builder()->CreateICmpEQ(lhs_code, rhs_code, "cmp_eq_tmp");
        return Builder()->CreateUIToFP(lhs_code, llvm::Type::getDoubleTy(*Context()), "bool_tmp");
    } else if (op == "<") {
        lhs_code = Builder()->CreateFCmpULE(lhs_code, rhs_code, "cmp_lt_tmp");
        return Builder()->CreateUIToFP(lhs_code, llvm::Type::getDoubleTy(*Context()), "bool_tmp");
    } else {
        throw CodegenError(fmt::format("Invalid binary operator, found '{}'", op).c_str());
    }
}

std::string CallExpr::toString() const {
    std::stringstream ss;
    for (const auto &ptr: args) {
        ss << ptr->toString();
        ss << ", ";
    }
    auto s = ss.str();
    // Remove last delimiter
    s = s.substr(0, s.length() - 2);
    return format("Call(callee={}, args=[{}])", callee, s);
}

llvm::Value *CallExpr::generateIR() const {
    // Look up function
    auto func = Module()->getFunction(callee);

    if (!func) {
        throw CodegenError(fmt::format("Unknown function '{}'", callee).c_str());
    }

    // Check for argument mismatch
    auto a = func->arg_size();
    auto b = args.size();
    if (a != b) {
        std::string msg = "Function '{}' requires {} arguments, given {}";
        throw CodegenError(fmt::format(msg, callee, a, b).c_str());
    }

    // Codegen for args
    std::vector<llvm::Value *> args_code;
    for (const auto &arg: args) {
        args_code.push_back(arg->generateIR());
        if (!args_code.back()) return nullptr;
    }

    return Builder()->CreateCall(func, args_code);
}

std::string Prototype::toString() const {
    // Concatenate all args
    auto arg_con = fmt::to_string(fmt::join(args.begin(), args.end(), ", "));
    return format("Proto(name={}, args=[{}])", name, arg_con);
}

llvm::Function *Prototype::generateIR() const {
    auto double_type = llvm::Type::getDoubleTy(*Context());

    // Make argument types
    // Firestorm only supports values that are doubles
    // Hence, arguments of all proto has the form (double, ...)
    std::vector<llvm::Type *> args_type{args.size(), double_type};

    // Make function type here
    // Similarly, the return type is double
    auto func_type = llvm::FunctionType::get(double_type, args_type, false);

    auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                       name, Module().get());

    // Set names for easy reference
    unsigned idx = 0;
    for (auto &arg: func->args()) {
        arg.setName(args[idx++]);
    }
    return func;
}

std::string Function::toString() const {
    auto p = proto->toString();
    auto b = body->toString();
    return format("Function(proto={}, body={})", p, b);
}

llvm::Value *Function::generateIR() const {
    // Check for existing function
    auto func = Module()->getFunction(proto->name);

    // If function not found, i.e. not yet declared, defined
    // then codegen its proto
    if (!func) func = proto->generateIR();

    // If codegen failed, idk man :v
    if (!func) return nullptr;

    // Check for existing definition
    if (!func->empty()) {
        auto msg = fmt::format("Function '{}' cannot be redefined", proto->name);
        throw CodegenError(msg.c_str());
    }

    // Create a basic block for function, i.e. function body
    // SetInsertPoint to specify that instructions shall be appended to block
    auto block = llvm::BasicBlock::Create(*Context(), "entry", func);
    Builder()->SetInsertPoint(block);

    // Record function arguments
    NamedValues().clear();
    for (auto &arg: func->args()) {
        NamedValues()[arg.getName().str()] = &arg;
    }

    // Implement function body
    if (auto body_code = body->generateIR()) {
        // Create return value
        Builder()->CreateRet(body_code);

        // Verify function well-formed-ness
        llvm::verifyFunction(*func);

        // Perform optimisation
        PassManager()->run(*func);

        return func;
    }

    // Otherwise, body codegen failed
    // then remove from module
    // This solves problems when functions are typed incorrectly in interpreter mode
    // allowing them to redefine it
    func->removeFromParent();
    return nullptr;
}

std::string IfExpr::toString() const {
    auto i = condition_clause->toString();
    auto t = then_clause->toString();
    auto e = else_clause->toString();
    return fmt::format("Conditional(if={}, then={}, else={})", i, t, e);
}

llvm::Value *IfExpr::generateIR() const {
    return nullptr;
}
