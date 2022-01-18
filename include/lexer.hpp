//
// Created by Nguyen Thai Binh on 16/1/22.
//

#ifndef FIRESTORM_LEXER_HPP
#define FIRESTORM_LEXER_HPP

#include <fmt/format.h>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "utility.hpp"

namespace {
    struct SourcePosition {
        long index = -1, lineno = -1, colno = -1;
    };

    std::vector<std::pair<std::string, std::regex>> &getRuleSet() {
        static std::vector<std::pair<std::string, std::regex>> rule_set;

        // Avoid multiple creation of rule set
        static bool already = false;
        if (already) return rule_set;
        else already = true;

        // I. Keywords
        // 1. If statement
        rule_set.emplace_back("IF", std::regex("^if"));
        rule_set.emplace_back("THEN", std::regex("^then"));
        rule_set.emplace_back("ELSE", std::regex("^else"));

        // 2. While loop
//    rule_set.emplace_back("WHILE", std::regex("^while"));
        // "then" is already present
        // "end" is already present

        // 3. Function declaration
        rule_set.emplace_back("DEFINE", std::regex("^define"));
        // "end" is already present

        // 4. External symbol
        rule_set.emplace_back("EXTERN", std::regex("^extern"));

        // II. Literals
        // 1. Numbers
        rule_set.emplace_back("NUMBER", std::regex(R"(^\d+(?:\.\d+)?)"));

        // III. Operators
        // 1. Arithmetic operators
        rule_set.emplace_back("PLUS", std::regex("^\\+"));
        rule_set.emplace_back("MINUS", std::regex("^-"));
        rule_set.emplace_back("TIMES", std::regex("^\\*"));
        rule_set.emplace_back("DIVIDE", std::regex("^/"));

        // 2. Comparison operators
        rule_set.emplace_back("EQL", std::regex("^=="));
        rule_set.emplace_back("LT", std::regex("^<"));

        // IV. Miscellaneous tokens
//    rule_set.emplace_back("EQUALS", std::regex("^="));
        rule_set.emplace_back("LPAREN", std::regex("^\\("));
        rule_set.emplace_back("RPAREN", std::regex("^\\)"));
//    rule_set.emplace_back("LCURLY", std::regex("^\\{"));
//    rule_set.emplace_back("RCURLY", std::regex("^\\}"));
        rule_set.emplace_back("COMMA", std::regex("^,"));
        rule_set.emplace_back("SEMICOLON", std::regex("^;"));
        rule_set.emplace_back("ID", std::regex("^[_a-zA-Z][_a-zA-Z0-9]*"));

        return rule_set;
    }
}

struct Token {
    std::string name;
    std::string value;
    SourcePosition position;

    [[nodiscard]]
    std::string toString() const;

    [[nodiscard]]
    inline bool isEOF() const { return name == "EOF"; };
};

struct Lexer;

struct TokenStream {
    const Lexer &lexer;
    const std::string source;
    long index = 0, lineno = 1, colno = 1;
    Token currentToken;

    TokenStream(const Lexer &l, std::string s) : lexer(l), source(std::move(s)) { updateSourcePos(); }

    Token getNextToken();

private:
    void updateSourcePos();
};

struct Lexer {
    const std::vector<std::pair<std::string, std::regex>> &rules;

    Lexer() : rules(getRuleSet()) {}

    [[nodiscard]]
    inline TokenStream lex(const std::string &input) const { return {*this, input}; }
};

struct LexingError : public FirestormError {
    explicit LexingError(const char *msg) : FirestormError(msg) {}
};

#endif //FIRESTORM_LEXER_HPP
