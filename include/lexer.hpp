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
    static const std::vector<std::pair<Type, std::regex>> &getRuleSet() {
        static std::vector<std::pair<Type, std::regex>> rule_set;

        // Avoid multiple creation of rule set
        static bool already = false;
        if (already) return rule_set;
        else already = true;

        // I. Keywords
        // Since these are keywords, the pattern needs to match only if there
        // are whitespaces after them
        // 1. If statement
        rule_set.emplace_back(Type::If, std::regex("^if(?=\\s+)"));
        rule_set.emplace_back(Type::Then, std::regex("^then(?=\\s+)"));
        rule_set.emplace_back(Type::Else, std::regex("^else(?=\\s+)"));

        // 2. While loop
//    rule_set.emplace_back(Type::While, std::regex("^while(?=\s+)"));
        // "then" is already present

        // 3. Function declaration
        rule_set.emplace_back(Type::Define, std::regex("^define(?=\\s+)"));

        // 4. External symbol
        rule_set.emplace_back(Type::Extern, std::regex("^extern(?=\\s+)"));

        // II. Literals
        // 1. Numbers
        rule_set.emplace_back(Type::Number, std::regex(R"(^\d+(?:\.\d+)?)"));

        // III. Operators
        // 1. Arithmetic operators
        rule_set.emplace_back(Type::Plus, std::regex("^\\+"));
        rule_set.emplace_back(Type::Minus, std::regex("^-"));
        rule_set.emplace_back(Type::Times, std::regex("^\\*"));
        rule_set.emplace_back(Type::Divide, std::regex("^/"));

        // 2. Comparison operators
        rule_set.emplace_back(Type::Equ, std::regex("^=="));
        rule_set.emplace_back(Type::Lt, std::regex("^<"));

        // IV. Miscellaneous tokens
//    rule_set.emplace_back("EQUALS", std::regex("^="));
        rule_set.emplace_back(Type::Lparen, std::regex("^\\("));
        rule_set.emplace_back(Type::Rparen, std::regex("^\\)"));
        rule_set.emplace_back(Type::Comma, std::regex("^,"));
        rule_set.emplace_back(Type::Semicolon, std::regex("^;"));
        rule_set.emplace_back(Type::Id, std::regex("^[_a-zA-Z][_a-zA-Z0-9]*"));

        return rule_set;
    }

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
