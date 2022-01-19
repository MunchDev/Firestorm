//
// Created by Nguyen Thai Binh on 16/1/22.
//
#include <algorithm>
#include <fmt/format.h>
#include <map>
#include <regex>

#include "lexer.hpp"

[[maybe_unused]]
std::string Token::toString() const {
    std::string msg = "Token(type={}, value={}, index={}, lineno={}, colno={})";
    return fmt::format(msg, getType(), value, position.index, position.lineno, position.colno);
}

const auto &getTypeName(Type type) {
    static std::map<Type, std::string> type_map{
            {Type::Eof, "EOF"},
            {Type::If, "IF"},
            {Type::Then, "THEN"},
            {Type::Else, "ELSE"},
//            {Type::While, "WHILE"},
            {Type::Define, "DEFINE"},
            {Type::Extern, "EXTERN"},
            {Type::Number, "NUMBER"},
            {Type::Plus, "PLUS"},
            {Type::Minus, "MINUS"},
            {Type::Times, "TIMES"},
            {Type::Divide, "DIVIDE"},
            {Type::Equ, "EQU"},
            {Type::Lt, "LT"},
//            {Type::Equals, "EQUALS"},
            {Type::Lparen, "LPAREN"},
            {Type::Rparen, "RPAREN"},
            {Type::Comma, "COMMA"},
            {Type::Id, "ID"},
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
    throw LexingError(fmt::format("Unexpected character at line {}, column {}", lineno, colno).c_str());
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
