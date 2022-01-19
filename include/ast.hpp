//
// Created by Nguyen Thai Binh on 17/1/22.
//

#ifndef FIRESTORM_AST_HPP
#define FIRESTORM_AST_HPP

#include "codegen.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Firestorm::AST {

    struct Expr {
        virtual ~Expr() = default;

        [[nodiscard]]
        virtual std::string toString() const = 0;

        [[nodiscard]]
        virtual llvm::Value *generateIR() const = 0;
    };

    using ExprPtr = std::unique_ptr<Expr>;

    struct NumberExpr : public Expr {
        double value;

        explicit NumberExpr(double v) : value(v) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    struct VariableExpr : public Expr {
        std::string name;

        explicit VariableExpr(std::string n) : name(std::move(n)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    struct IfExpr : public Expr {
        ExprPtr condition_clause, then_clause, else_clause;

        IfExpr(ExprPtr c, ExprPtr t, ExprPtr e)
                : condition_clause(std::move(c)), then_clause(std::move(t)), else_clause(std::move(e)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    struct BinaryExpr : public Expr {
        ExprPtr lhs;
        std::string op;
        ExprPtr rhs;

        BinaryExpr(ExprPtr l, std::string o, ExprPtr r) : lhs(std::move(l)), op(std::move(o)), rhs(std::move(r)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    struct CallExpr : public Expr {
        std::string callee;
        std::vector<ExprPtr> args;

        CallExpr(std::string c, std::vector<ExprPtr> a) : callee(std::move(c)), args(std::move(a)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    struct Prototype : public Expr {
        std::string name;
        std::vector<std::string> args;

        Prototype(std::string n, std::vector<std::string> a) : name(std::move(n)), args(std::move(a)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Function *generateIR() const override;
    };

    using ProtoPtr = std::unique_ptr<Prototype>;

    struct Function : public Expr {
        ProtoPtr proto;
        ExprPtr body;

        Function(ProtoPtr p, ExprPtr b) : proto(std::move(p)), body(std::move(b)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    using FunctionPtr = std::unique_ptr<Function>;

    CodeGenerator &getCodegen();
}
#endif //FIRESTORM_AST_HPP
