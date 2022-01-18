//
// Created by Nguyen Thai Binh on 18/1/22.
//
#include "play.hpp"

void loop_output_ir() {
    // Initialisation
    Lexer lexer;
    Token previousToken;        // Continue the previous stream

    std::string input;

    while (true) {
        std::cout.flush() << "Input> ";

        std::getline(std::cin, input);
        if (input == "=exit") break;
        try {
            // Tokenize input
            auto stream = lexer.lex(input);

            // Load previous token
            stream.currentToken = previousToken;

            // Parse token stream
            auto program = Parser(stream).parse();

            // Save last token
            previousToken = stream.currentToken;

            // Print IR
            for (const auto& stmt: program) {
                auto IR = stmt->generateIR();
                IR->print(llvm::outs());
                llvm::outs() << '\n';
            }
        } catch (const FirestormError &error) {
            llvm::outs() << "Error: " << error.what() << "\n";
        }
    }
    // Print the entire module
    getCodegen().module->print(llvm::outs(), nullptr);
}
