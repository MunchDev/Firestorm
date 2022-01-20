//
// Created by Nguyen Thai Binh on 19/1/22.
//
#include "Firestorm/custom_exceptions.hpp"

#include <exception>
#include <fmt/format.h>

namespace Firestorm::Utility {
    FirestormError::FirestormError(const std::string& msg) : std::runtime_error(msg) {}

    LexerError::LexerError(const std::string &msg) : FirestormError(msg), std::runtime_error(msg) {}

    ParserError::ParserError(const std::string &msg) : FirestormError(msg), std::runtime_error(msg) {}

    CodegenError::CodegenError(const std::string &msg) : FirestormError(msg), std::runtime_error(msg) {}
}
