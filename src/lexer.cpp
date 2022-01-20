//
// Created by Nguyen Thai Binh on 16/1/22.
//
#include "Firestorm/custom_exceptions.hpp"
#include "Firestorm/lexer.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <map>
#include <regex>

namespace Firestorm::Lexing {
    [[maybe_unused]]
    std::string Token::toString() const {
        std::string msg = "Token(type={}, value={}, index={}, lineno={}, colno={})";
        return fmt::format(msg, getType(), value, position.index, position.lineno, position.colno);
    }

    const std::vector<std::pair<Type, std::regex>> &getRuleSet() {
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

    const auto &getTypeName(Type type) {
        static std::map<Type, std::string> type_map{
                {Type::Eof,    "EOF"},
                {Type::If,     "IF"},
                {Type::Then,   "THEN"},
                {Type::Else,   "ELSE"},
//            {Type::While, "WHILE"},
                {Type::Define, "DEFINE"},
                {Type::Extern, "EXTERN"},
                {Type::Number, "NUMBER"},
                {Type::Plus,   "PLUS"},
                {Type::Minus,  "MINUS"},
                {Type::Times,  "TIMES"},
                {Type::Divide, "DIVIDE"},
                {Type::Equ,    "EQU"},
                {Type::Lt,     "LT"},
//            {Type::Equals, "EQUALS"},
                {Type::Lparen, "LPAREN"},
                {Type::Rparen, "RPAREN"},
                {Type::Comma,  "COMMA"},
                {Type::Id,     "ID"},
        };
        return type_map[type];
    }

    std::string Token::getType() const {
        return getTypeName(type);
    }

    Token TokenStream::getNextToken() {
        // Check if finished, return EOF token
        if (index == source.length()) return currentToken = {Type::Eof, "EOF", {index, lineno, colno}};

        // Get substring, ignoring already lexed
        auto sub_str = source.substr(index, source.length() - index);

        // Iterate over all rules
        std::smatch match_info;
        for (const auto &rule: lexer.rules) {
            // Search substring against rule
            if (std::regex_search(sub_str, match_info, rule.second)) {
                // Get the matched string
                auto matched = sub_str.substr(match_info.position(), match_info.length());

                // Craft token
                currentToken = {rule.first, matched, {index, lineno, colno}};

                // Update position
                updateSourcePos();

                return currentToken;
            }
        }
        // Throw an error when source doesn't match any rules
        throw Utility::getError(Utility::LE, "[{}:{}] Unknown character '{}'", lineno, colno, sub_str[0]);
    }

    void TokenStream::updateSourcePos() {
        // Increment index and colno by the length of the token
        auto l = (long) currentToken.value.length();
        index += l;
        colno += l;

        // Find the first character (from index) that is not a whitespace
        auto first_non_ws = source.find_first_not_of(" \n", index);

        // If there aren't any whitespaces, return
        if (first_non_ws == index) return;

        // Check if end of source, i.e. the rest are whitespaces
        if (first_non_ws == std::string::npos) {
            // Set line, column and index
            index = (long) source.length();
            lineno = std::count(source.begin(), source.end(), '\n') + 1;
            colno = (long) (source.length() - source.rfind('\n') + 1);
            return;
        }

        // Substring to match against (from index to first non-whitespace)
        auto sub_str = source.substr(index, first_non_ws - index);

        // Increment index by length of whitespaces
        index += (long) sub_str.length();

        // Check if there are newlines in substring
        if (sub_str.find('\n') != std::string::npos) {
            // Count the number of newline(s)
            auto nl_count = std::count(sub_str.begin(), sub_str.end(), '\n');

            // Increment line number by the number of newline(s)
            lineno += nl_count;

            // Find last newline in substring
            auto last_nl = sub_str.rfind('\n');

            // Set column number to the number of whitespaces after the last newline
            colno = (long) (sub_str.length() - last_nl);
            return;
        }

        // Otherwise, if there aren't any newlines, update
        // column according to length of substring
        colno += (long) sub_str.length();
    }
}
