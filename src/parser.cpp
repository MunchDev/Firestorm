//
// Created by Nguyen Thai Binh on 17/1/22.
//
#include "ast.hpp"
#include "custom_exceptions.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <memory>
#include <vector>

namespace Firestorm::Parsing {
    Utility::FirestormError getError(const std::string& msg, const Lexing::Token& token) {
        auto l = token.position.lineno;
        auto c = token.position.colno;
        auto v = token.value;
        return Utility::getError(msg, l, c, v);
    }

    int Parser::getOperatorPrecedence() {
        auto p = precedence_table[stream.currentToken.value];
        if (p > 0) return p;
        return -1;
    }

    ExprPtr Parser::parseIfExpr() {
        // Consume IF token
        stream.getNextToken();

        // Parse condition_clause
        auto cond = parseExpr();
        if (!cond) return nullptr;

        // Check for and consume THEN token
        if (stream.currentToken.type != Lexing::Type::Then) {
            throw getError("[{}:{}] Expected 'then', found '{}'", stream.currentToken);
        }
        stream.getNextToken();

        // Parse then_clause
        auto then = parseExpr();
        if (!then) return nullptr;

        // Check for and consume ELSE token
        if (stream.currentToken.type != Lexing::Type::Else) {
            throw getError("[{}:{}] Expected 'else', found '{}'", stream.currentToken);
        }
        stream.getNextToken();

        // Parse then_clause
        auto _else = parseExpr();
        if (!_else) return nullptr;

        return std::make_unique<AST::IfExpr>(std::move(cond), std::move(then), std::move(_else));
    }

    ExprPtr Parser::parseNumExpr() {
        // Convert token value to double
        auto value = std::strtod(stream.currentToken.value.c_str(), nullptr);

        // Consume the token
        stream.getNextToken();
        return std::move(std::make_unique<AST::NumberExpr>(value));
    }

    ExprPtr Parser::parseParenExpr() {
        // Consume LPAREN token
        stream.getNextToken();

        // Parse expr
        auto value = parseExpr();
        if (!value) return nullptr;

        // Check for matching RPAREN
        if (stream.currentToken.type != Lexing::Type::Rparen) {
            throw getError("[{}:{}] Expected ')', found '{}'", stream.currentToken);
        }

        // Consume RPAREN token
        stream.getNextToken();
        return value;
    }

    ExprPtr Parser::parseIdExpr() {
        // Get identifier type
        std::string id = stream.currentToken.value;

        // Check if next token is LPAREN
        if (stream.getNextToken().type != Lexing::Type::Lparen) {
            // If not, then simple variable
            return std::make_unique<AST::VariableExpr>(id);
        }

        // If reach here, then it is function call
        // Consume LPAREN and check for RPAREN
        std::vector<ExprPtr> args;
        if (stream.getNextToken().type != Lexing::Type::Rparen) {
            // Not RPAREN -> Function call with arguments
            // Loop to get all arguments
            while (true) {
                auto arg = parseExpr();
                if (!arg) return nullptr;
                args.push_back(std::move(arg));

                // Check for RPAREN
                // then break this loop
                if (stream.currentToken.type == Lexing::Type::Rparen) break;

                // Check if not comma then raise error
                if (stream.currentToken.type != Lexing::Type::Comma) {
                    throw getError("[{}:{}] Expected ')' or ',', found '{}'", stream.currentToken);
                }

                // Consume COMMA token
                stream.getNextToken();
            }
        }

        // Consume RPAREN
        stream.getNextToken();
        return std::make_unique<AST::CallExpr>(id, std::move(args));
    }

    ExprPtr Parser::parsePrimary() {
        // This method is self-explanatory
        auto name = stream.currentToken.type;
        if (name == Lexing::Type::Number) {
            return parseNumExpr();
        } else if (name == Lexing::Type::Lparen) {
            return parseParenExpr();
        } else if (name == Lexing::Type::Id) {
            return parseIdExpr();
        } else if (name == Lexing::Type::If) {
            return parseIfExpr();
        } else {
            throw getError("[{}:{}] Expected an expression, found '{}'", stream.currentToken);
        }
    }

    ExprPtr Parser::parseExpr() {
        // Get LHS
        auto lhs = parsePrimary();
        if (!lhs) return nullptr;
        auto v = parseBinOpRHS(0, std::move(lhs));
        return v;
    }

    ExprPtr Parser::parseBinOpRHS(int exprPre, ExprPtr lhs) {
        // Keep looping to parse the entire expression
        while (true) {
            // Get token exprPre
            auto currentPre = getOperatorPrecedence();

            // If token is not a BinOp, currentPre is -1
            // If token is a BinOp that has strictly smaller
            // exprPre than current exprPre, then return LHS
            if (currentPre < exprPre) return lhs;

            // Otherwise, currentPre is indeed a BinOp and will be included
            // in this parsing round
            auto currentOp = stream.currentToken.value;

            // Consume the operator
            stream.getNextToken();

            // Parse the primary associated with this BinOp
            auto rhs = parsePrimary();
            if (!rhs) return nullptr;

            // Same thing as the currentPre, get nextPre
            auto nextPre = getOperatorPrecedence();
            if (currentPre < nextPre) {
                // In this case, rhs will be the new 'lhs' of the next BinOp
                // As such, we will recursively get rhs until the BinOp followed it
                // has lower precedence than currentPre
                // The +1 is because we parse from left to right, so if two BinOps
                // are the same, we parse the previous first
                rhs = parseBinOpRHS(exprPre + 1, std::move(rhs));
                if (!rhs) return nullptr;
            }

            // Now that we have both lhs and rhs and a BinOp to combine them
            // we can combine them and continue parsing the rest of the expression
            lhs = std::make_unique<AST::BinaryExpr>(std::move(lhs), currentOp, std::move(rhs));
        }
    }

    ProtoPtr Parser::parseProto() {
        // Assert that current token is an ID
        if (stream.currentToken.type != Lexing::Type::Id) {
            throw getError("[{}:{}] Expected name in prototype, found '{}'", stream.currentToken);
        }

        // Get function type, i.e. the ID
        auto func_name = stream.currentToken.value;

        // Consume ID and check for LPAREN
        if (stream.getNextToken().type != Lexing::Type::Lparen) {
            throw getError("[{}:{}] Expected '(', found '{}'", stream.currentToken);
        }

        // Now parse ids
        std::vector<std::string> ids;

        // Check for standalone id
        // ids := ID
        if (stream.getNextToken().type == Lexing::Type::Id) {
            ids.push_back(stream.currentToken.value);

            // Now check for more COMMA ID pair
            while (stream.getNextToken().type == Lexing::Type::Comma) {
                // Consume COMMA
                stream.getNextToken();

                // Check that an ID follows
                if (stream.currentToken.type != Lexing::Type::Id) {
                    throw getError("[{}:{}] Expected ID, found '{}'", stream.currentToken);
                }

                // Add ID to list
                ids.push_back(stream.currentToken.value);
            }
        }

        // At the end of argument lists, check for RPAREN
        if (stream.currentToken.type != Lexing::Type::Rparen) {
            throw getError("[{}:{}] Expected ')', found '{}'", stream.currentToken);
        }

        // Now that everything is good to go, consume RPAREN
        stream.getNextToken();
        return std::make_unique<AST::Prototype>(func_name, ids);
    }

    FunctionPtr Parser::parseDefineStmt() {
        // Consume DEFINE token
        stream.getNextToken();

        // Parse proto
        auto proto = parseProto();
        if (!proto) return nullptr;

        if (auto body = parseExpr()) {
            return std::make_unique<AST::Function>(std::move(proto), std::move(body));
        }
        return nullptr;
    }

    ProtoPtr Parser::parseExternStmt() {
        // Consume EXTERN token
        stream.getNextToken();
        return parseProto();
    }

    ExprPtr Parser::parseTLOStmt() {
        // Check for EXTERN and DEFINE keyword
        if (stream.currentToken.type == Lexing::Type::Extern) {
            return parseExternStmt();
        } else if (stream.currentToken.type == Lexing::Type::Define) {
            return parseDefineStmt();
        } else {
            throw getError("[{}:{}] Expected 'extern' or 'define', found '{}'", stream.currentToken);
        }
    }

    ExprPtr Parser::parseOtrStmt() {
        // Self-explanatory
        return parseExpr();
    }

    ExprPtr Parser::parseStmt() {
        // Self-explanatory
        if (stream.currentToken.type == Lexing::Type::Extern || stream.currentToken.type == Lexing::Type::Define) {
            return parseTLOStmt();
        }
        return parseOtrStmt();
    }

    std::vector<ExprPtr> Parser::parseStmts() {
        std::vector<ExprPtr> stmts;

        // Keep looping to get all statement
        auto stmt = parseStmt();
        while (stmt) {
            // Check semicolon
            if (stream.currentToken.type != Lexing::Type::Semicolon) {
                throw getError("[{}:{}] Expected ';' after statement, found '{}'", stream.currentToken);
            }

            // Add statement to stmts
            stmts.push_back(std::move(stmt));

            // Consume SEMICOLON and check for null token , i.e. end of source
            if (stream.getNextToken().type == Lexing::Type::Eof) break;

            // Get next statement (if any)
            stmt = parseStmt();
        }

        return stmts;
    }

    std::vector<ExprPtr> Parser::parseProgram() {
        return parseStmts();
    }

    std::vector<ExprPtr> Parser::parse() {
        // Get first token
        stream.getNextToken();
        // Check if source is empty, i.e. null token
        if (stream.currentToken.type == Lexing::Type::Eof) return {};
        return parseProgram();
    }
}
