//
// Created by Nguyen Thai Binh on 17/1/22.
//

#ifndef FIRESTORM_CUSTOM_EXCEPTIONS_HPP
#define FIRESTORM_CUSTOM_EXCEPTIONS_HPP

#include <exception>
#include <fmt/format.h>
#include <string>

namespace Firestorm::Utility {
    /// @brief Base class for all exceptions thrown by Firestorm.
    ///
    /// @note This exception is not to be thrown directly. Use getError() instead.
    struct FirestormError : virtual public std::runtime_error {
        explicit FirestormError(const std::string &msg);
    };

    /// @brief Subclass of FirestormError. Thrown when an error occurred while lexing Firestorm code.
    ///
    /// @note This exception is not to be thrown directly. Use getError() instead.
    struct LexerError : FirestormError {
        explicit LexerError(const std::string &msg);
    };

    /// @brief Subclass of FirestormError. Thrown when an error occurred while parsing Firestorm code.
    ///
    /// @note This exception is not to be thrown directly. Use getError() instead.
    struct ParserError : FirestormError {
        explicit ParserError(const std::string &msg);
    };

    /// @brief Subclass of FirestormError. Thrown when an error occurred while emitting LLVM IR for Firestorm code.
    ///
    /// @note This exception is not to be thrown directly. Use getError() instead.
    struct CodegenError : FirestormError {
        explicit CodegenError(const std::string &msg);
    };

    enum ErrorType {
        FE, // FirestormError
        LE, // LexerError
        PE, // ParserError
        CE, // CodegenError
    };

    /// @brief Get an instance of FirestormError or its subclasses with a message and arguments
    ///
    /// @tparam T Any value convertible to std::string
    ///
    /// @param type The type of error to return
    ///
    /// @param msg A message accompanying the error
    ///
    /// @param args Variadic arguments, to be passed to fmt::format()
    ///
    /// @return An instance of FirestormError or one of its subclasses
    template<class... T>
    FirestormError getError(ErrorType type, std::string msg, T &&... args) {
        msg = fmt::format(msg, args...);
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
}


#endif //FIRESTORM_CUSTOM_EXCEPTIONS_HPP
