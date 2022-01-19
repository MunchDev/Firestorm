//
// Created by Nguyen Thai Binh on 19/1/22.
//
#include "custom_exceptions.hpp"

#include <exception>
#include <fmt/format.h>

namespace Firestorm::Utility {
    FirestormError::FirestormError(const std::string& msg) : std::runtime_error(msg) {}

    LexerError::LexerError(const std::string &msg) : FirestormError(msg), std::runtime_error(msg) {}

    ParserError::ParserError(const std::string &msg) : FirestormError(msg), std::runtime_error(msg) {}

    CodegenError::CodegenError(const std::string &msg) : FirestormError(msg), std::runtime_error(msg) {}

    template<class... T>
    FirestormError getError(ErrorType type, const std::string& msg, T... args) {
        return getError(type, fmt::format(msg, args...));
    }

    FirestormError getError(ErrorType type, const std::string &msg) {
        switch (type) {
            case FE:
                return FirestormError(msg);
            case LE:
                return LexerError(msg);
            case PE:
                return ParserError(msg);
            case CE:
                return CodegenError(msg);
        }
    }

    template<class... T>
    FirestormError getError(const std::string& msg, T... args) {
        return getError(FE, msg, args...);
    }

    FirestormError getError(const std::string& msg) {
        return getError(FE, msg);
    }
}
