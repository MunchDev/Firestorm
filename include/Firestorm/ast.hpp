//
// Created by Nguyen Thai Binh on 17/1/22.
//

#ifndef FIRESTORM_AST_HPP
#define FIRESTORM_AST_HPP

#include "codegen.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Firestorm::AST {
    /// @brief Base class for all AST nodes.
    struct Expr {
        virtual ~Expr() = default;

        [[nodiscard]]
        virtual std::string toString() const = 0;

        [[nodiscard]]
        virtual llvm::Value *generateIR() const = 0;
    };

    /// @brief Quick using-directive for convenience
    using ExprPtr = std::unique_ptr<Expr>;

    /// @brief Contains a single double-precision floating-point number.
    struct NumberExpr : public Expr {
        double value;

        explicit NumberExpr(double v) : value(v) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    /// @brief Contains a single named variable.
    struct VariableExpr : public Expr {
        std::string name;

        explicit VariableExpr(std::string n) : name(std::move(n)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    /// @brief Contains a single conditional expression.
    struct IfExpr : public Expr {
        ExprPtr condition_clause, then_clause, else_clause;

        IfExpr(ExprPtr c, ExprPtr t, ExprPtr e)
                : condition_clause(std::move(c)), then_clause(std::move(t)), else_clause(std::move(e)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    /// @brief Contains a single for-loop expression.

    struct ForExpr : public Expr {
        std::string varName;
        ExprPtr start, end, step, body;

        ForExpr(std::string v, ExprPtr s, ExprPtr e, ExprPtr s1, ExprPtr b) : varName(std::move(v)),
                                                                              start(std::move(s)), end(std::move(e)),
                                                                              step(std::move(s1)), body(std::move(b)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    /// @brief Contains a single binary expression. Can be nested.
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

    /// @brief Contains a single function call.
    struct CallExpr : public Expr {
        std::string callee;
        std::vector<ExprPtr> args;

        CallExpr(std::string c, std::vector<ExprPtr> a) : callee(std::move(c)), args(std::move(a)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    /// @brief Contains a single function prototype.
    struct Prototype : public Expr {
        std::string name;
        std::vector<std::string> args;

        Prototype(std::string n, std::vector<std::string> a) : name(std::move(n)), args(std::move(a)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Function *generateIR() const override;
    };

    /// @brief Quick using-directive for convenience
    using ProtoPtr = std::unique_ptr<Prototype>;

    /// @brief Contains a single function definition.
    struct Function : public Expr {
        ProtoPtr proto;
        ExprPtr body;

        Function(ProtoPtr p, ExprPtr b) : proto(std::move(p)), body(std::move(b)) {}

        [[nodiscard]]
        std::string toString() const override;

        [[nodiscard]]
        llvm::Value *generateIR() const override;
    };

    /// @brief Quick using-directive for convenience
    using FunctionPtr = std::unique_ptr<Function>;

    /// @return Get an instance of CodeGenerator
    CodeGenerator &getCodegen();
}
#endif //FIRESTORM_AST_HPP
