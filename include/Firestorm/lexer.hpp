//
// Created by Nguyen Thai Binh on 16/1/22.
//
#ifndef FIRESTORM_LEXER_HPP
#define FIRESTORM_LEXER_HPP

#include "custom_exceptions.hpp"

#include <fmt/format.h>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace Firestorm::Lexing {
    /// @brief Represents different types of token in Firestorm code.
    enum class Type {
        Eof,
        If,
        Then,
        Else,
//    While,
        Define,
        Extern,
        Number,
        Plus,
        Minus,
        Times,
        Divide,
        Equ,
        Lt,
//    Equals,
        Lparen,
        Rparen,
        Comma,
        Semicolon,
        Id,
    };

    /// @brief Stores source position of the first character in each token.
    struct SourcePosition {
        long index = -1, lineno = -1, colno = -1;
    };

    /// Creates lexing rule sets for Firestorm's lexer.
    /// @return The regular expression ruleset for Firestorm
    const std::vector<std::pair<Type, std::regex>> &getRuleSet();

    /// @brief Consists of a single token extracted from source.
    struct Token {
        Type type{Type::Eof};
        std::string value;
        SourcePosition position{};

        /// @return String representation of Token
        [[nodiscard]]
        std::string toString() const;

        /// @return String representation of token's type
        [[nodiscard]]
        std::string getType() const;
    };

    struct Lexer;

    /// @brief Can be used to retrieve tokens one-by-one from source;
    struct TokenStream {
        const Lexer &lexer;
        const std::string& source;
        long index = 0, lineno = 1, colno = 1;
        Token currentToken;

        TokenStream(const Lexer &l, const std::string& s) : lexer(l), source(s) { updateSourcePos(); }

        /// @return Next token in source
        Token getNextToken();

    private:
        /// @brief Updates index, line number and column number to the next character in source.
        void updateSourcePos();
    };

    /// @brief Represents the rule set of Firestorm and acts as a wrapper of TokenStream
    struct Lexer {
        const std::vector<std::pair<Type, std::regex>> &rules;

        Lexer() : rules(getRuleSet()) {}

        /// \param input Firestorm source to be parsed
        /// \return An instance of TokenStream
        [[nodiscard]]
        inline TokenStream lex(const std::string &input) const { return {*this, input}; }
    };
}
#endif //FIRESTORM_LEXER_HPP
