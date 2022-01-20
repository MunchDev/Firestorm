//
// Created by Nguyen Thai Binh on 17/1/22.
//
#include "Firestorm/ast.hpp"
#include "Firestorm/codegen.hpp"
#include "Firestorm/custom_exceptions.hpp"

#include <fmt/format.h>
#include <sstream>
#include <llvm/IR/Verifier.h>

namespace Firestorm::AST {
    using fmt::format;

    CodeGenerator &getCodegen() {
        static CodeGenerator codegen;
        return codegen;
    }

    auto &Context() {
        return getCodegen().context;
    }

    auto &Builder() {
        return getCodegen().builder;
    }

    auto &Module() {
        return getCodegen().module;
    }

    auto &Optimiser() {
        return getCodegen().optimiser;
    }

    auto &NamedValues() {
        return getCodegen().namedValues;
    }

    const auto &DoubleType() {
        static auto t = llvm::Type::getDoubleTy(Context());
        return t;
    }

    std::string NumberExpr::toString() const {
        return format("Number({})", value);
    }

    llvm::Value *NumberExpr::generateIR() const {
        return llvm::ConstantFP::get(Context(), llvm::APFloat(value));
    }

    std::string VariableExpr::toString() const {
        return format("Variable({})", name);
    }

    llvm::Value *VariableExpr::generateIR() const {
        // Look up if variable declared
        auto value = NamedValues()[name];
        if (!value) {
            throw Utility::getError(Utility::CE, "Unknown variable '{}'", name);
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
            return Builder().CreateFAdd(lhs_code, rhs_code, "add_tmp");
        else if (op == "-")
            return Builder().CreateFSub(lhs_code, rhs_code, "sub_tmp");
        else if (op == "*")
            return Builder().CreateFMul(lhs_code, rhs_code, "mul_tmp");
        else if (op == "/")
            return Builder().CreateFDiv(lhs_code, rhs_code, "div_tmp");
        else if (op == "==") {
            lhs_code = Builder().CreateICmpEQ(lhs_code, rhs_code, "cmp_eq_tmp");
            return Builder().CreateUIToFP(lhs_code, DoubleType(), "bool_tmp");
        } else if (op == "<") {
            lhs_code = Builder().CreateFCmpULE(lhs_code, rhs_code, "cmp_lt_tmp");
            return Builder().CreateUIToFP(lhs_code, DoubleType(), "bool_tmp");
        } else {
            throw Utility::getError(Utility::CE, "Invalid binary operator, found '{}'", op);
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
        auto func = Module().getFunction(callee);

        if (!func) {
            throw Utility::getError(Utility::CE, "Unknown function '{}'", callee);
        }

        // Check for argument mismatch
        auto a = func->arg_size();
        auto b = args.size();
        if (a != b) {
            throw Utility::getError(Utility::CE, "Function '{}' requires {} arguments, given {}", callee, a, b);
        }

        // Codegen for args
        std::vector<llvm::Value *> args_code;
        for (const auto &arg: args) {
            args_code.push_back(arg->generateIR());
            if (!args_code.back()) return nullptr;
        }

        return Builder().CreateCall(func, args_code);
    }

    std::string Prototype::toString() const {
        // Concatenate all args
        auto arg_con = fmt::to_string(fmt::join(args.begin(), args.end(), ", "));
        return format("Proto(name={}, args=[{}])", name, arg_con);
    }

    llvm::Function *Prototype::generateIR() const {
        // Make argument types
        // Firestorm only supports values that are doubles
        // Hence, arguments of all proto has the form (double, ...)
        std::vector<llvm::Type *> args_type{args.size(), DoubleType()};

        // Make function type here
        // Similarly, the return type is double
        auto func_type = llvm::FunctionType::get(DoubleType(), args_type, false);

        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                           name, Module());

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
        auto func = Module().getFunction(proto->name);

        // If function not found, i.e. not yet declared, defined
        // then codegen its proto
        if (!func) func = proto->generateIR();

        // If codegen failed, idk man :v
        if (!func) return nullptr;

        // Check for existing definition
        if (!func->empty()) {
            throw Utility::getError(Utility::CE, "Function '{}' cannot be redefined", proto->name);
        }

        // Create a basic block for function, i.e. function body
        // SetInsertPoint to specify that instructions shall be appended to block
        auto block = llvm::BasicBlock::Create(Context(), "entry", func);
        Builder().SetInsertPoint(block);

        // Record function arguments
        NamedValues().clear();
        for (auto &arg: func->args()) {
            NamedValues()[arg.getName().str()] = &arg;
        }

        // Implement function body
        if (auto body_code = body->generateIR()) {
            // Create return value
            Builder().CreateRet(body_code);

            // Verify function well-formed-ness
            llvm::verifyFunction(*func);

            // Perform optimisation
            // Notes: Temporary remove optimiser since its API is changing
            // and no one knows how to use the new one.
            Optimiser().passManager.run(*func);

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
        // codegen condition_clause
        auto cond_code = condition_clause->generateIR();
        if (!cond_code) return nullptr;

        // Convert cond_code to bool by comparing with zero
        // Here we utilise NumberExpr
        auto zero_code = NumberExpr(0).generateIR();
        cond_code = Builder().CreateFCmpONE(cond_code, zero_code, "if_cond");

        auto func = Builder().GetInsertBlock()->getParent();

        // Generate blocks for then, else, if_cont (merging then and else)
        auto then_block = llvm::BasicBlock::Create(Context(), "then", func);
        auto else_block = llvm::BasicBlock::Create(Context(), "else");
        auto cont_block = llvm::BasicBlock::Create(Context(), "if_cont");

        // Create conditional branch
        Builder().CreateCondBr(cond_code, then_block, else_block);

        // codegen then_code to insert to then_block
        Builder().SetInsertPoint(then_block);
        auto then_code = then_clause->generateIR();
        if (!then_code) return nullptr;

        // Make cont_block a branch from then_block
        // This is necessary because later, else_block will also branch to cont_block
        // NOTE: LLVM strictly require all branch to terminate explicitly
        Builder().CreateBr(cont_block);

        // codegen of then_clause could change block
        // Hence, we need to retrieve it
        then_block = Builder().GetInsertBlock();

        // codegen else_code to insert to else_code
        // But first add else_block to func
        func->getBasicBlockList().push_back(else_block);
        Builder().SetInsertPoint(else_block);
        auto else_code = else_clause->generateIR();
        if (!else_code) return nullptr;

        // Branch to cont_block (similar to above)
        Builder().CreateBr(cont_block);

        // codegen of else_clause could change block
        // Hence, we need to retrieve it
        else_block = Builder().GetInsertBlock();

        // Now insert cont_block into function
        func->getBasicBlockList().push_back(cont_block);
        Builder().SetInsertPoint(cont_block);

        // Make PHI node
        auto phi = Builder().CreatePHI(DoubleType(), 2, "if_tmp");
        phi->addIncoming(then_code, then_block);
        phi->addIncoming(else_code, else_block);
        return phi;
    }

    std::string ForExpr::toString() const {
        auto s = start->toString();
        auto e = end->toString();
        auto s1 = step->toString();
        auto b = body->toString();

        return fmt::format("ForExpr(var={}, start={}, end={}, step={}, body={})", varName, s, e, s1, b);
    }

    llvm::Value *ForExpr::generateIR() const {
        // codegen start value
        auto start_code = start->generateIR();
        if (!start_code) return nullptr;

        // Get pre-loop block
        auto pre_entry_block = Builder().GetInsertBlock();

        // Get parent function
        auto func = pre_entry_block->getParent();

        // Create loop block
        auto loop_block = llvm::BasicBlock::Create(Context(), "loop", func);

        // Create fall through pre-entry block to loop block
        Builder().CreateBr(loop_block);

        // Set insert point to loop block to put instructions there
        Builder().SetInsertPoint(loop_block);

        // Create PHI node
        // As loop block can come from pre-entry block as well as itself
        auto variable = Builder().CreatePHI(DoubleType(), 2, varName);
        variable->addIncoming(start_code, pre_entry_block);

        // Add loop variable to symbol table
        // Save the existing if any
        auto existing_value = NamedValues()[varName];
        NamedValues()[varName] = variable;

        // codegen body expression
        // We don't have to assign it to a variable because it's not needed
        // and insert point is already points to loop block
        if (!body->generateIR()) return nullptr;

        // codegen step value
        // If there isn't one (since it's optional), set it to default value of 1
        llvm::Value *step_code;
        if (step) {
            step_code = step->generateIR();
            if (!step_code) return nullptr;
        } else {
            step_code = NumberExpr(1.0).generateIR();
        }

        auto next_variable = Builder().CreateFAdd(variable, step_code, "next_" + varName);

        // codegen end condition
        auto end_code = end->generateIR();
        if (!end_code) return nullptr;

        // convert end condition to bool by comparing non-equal to 0.0
        end_code = Builder().CreateFCmpONE(end_code, NumberExpr(0.0).generateIR(), "loop_cond");

        // create after loop block
        auto loop_end_block = Builder().GetInsertBlock();
        auto after_loop_block = llvm::BasicBlock::Create(Context(), "after_loop", func);

        // create conditional branch based on end code
        // If true, go back to loop block, otherwise, go to after loop block
        Builder().CreateCondBr(end_code, loop_block, after_loop_block);

        // Set insert point to after block so any new code will go there
        Builder().SetInsertPoint(after_loop_block);

        // Add next variable as the second entry of PHI node
        variable->addIncoming(next_variable, loop_end_block);

        // Restore existing variable saved earlier
        if (existing_value) NamedValues()[varName] = existing_value;
        else NamedValues().erase(varName);

        // Set return value for for-loop
        // For now, it is set to default of 0.0
        return NumberExpr(0.0).generateIR();
    }
}
