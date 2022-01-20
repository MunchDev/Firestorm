//
// Created by Nguyen Thai Binh on 18/1/22.
//
#include "Firestorm/ast.hpp"
#include "Firestorm/custom_exceptions.hpp"
#include "Firestorm/lexer.hpp"
#include "Firestorm/parser.hpp"
#include "Firestorm/frontend.hpp"

#include <iostream>

namespace Firestorm::Frontend {
    void Interpreter::run() {
        // Initialisation
        Firestorm::Lexing::Lexer lexer;
        Firestorm::Lexing::Token previousToken;        // Continue the previous stream

        std::string input;

        while (true) {
            llvm::outs() << "Input> ";

            std::getline(std::cin, input);
            if (input == "=exit") break;
            try {
                // Tokenize input
                auto stream = lexer.lex(input);

                // Load previous token
                stream.currentToken = previousToken;

                // Parse token stream
                auto program = Firestorm::Parsing::Parser(stream).parse();

                // Save last token
                previousToken = stream.currentToken;

                // Print IR
                for (const auto &stmt: program) {
                    auto IR = stmt->generateIR();
                    IR->print(llvm::outs());
                    llvm::outs() << '\n';
                }
            } catch (const Firestorm::Utility::FirestormError &error) {
                llvm::outs() << "Error: " << error.what() << "\n";
            }
        }
        // Print the entire module
        Firestorm::AST::getCodegen().module.print(llvm::outs(), nullptr);
    }
}
