# Firestorm

Firestorm is a proof-of-concept programming language created to empower users
to implement the language themselves.

## Features

- `Firestorm` contains minimal amount of elements to be Turing-complete, while
being highly readable.

- `Firestorm` is so simple that it doesn't employ statements: *Everything is an
expression.* It's so simple that the entire EBNF grammar can be written in *less
than 50 lines!*

- `Firestorm` uses LLVM as its backend and features both a JIT-enabled interpreter
or a static compiler.

- `Firestorm` can be compiled into LLVM IR and bitcode, allowing you to experiment
with LLVM's extensive suite.

- `Firestorms` integrates C++ standard libraries (if needed) so you don't have to
reinvent the wheels.

## Installation

The project uses CMake to build. You'll need CMake with a minimum version of `3.21`.

Additionally, you'll need the following installed on your machine:

- `fmt` (version `8.0` or compatible)

- `llvm` (version `13.0` or compatible)

Standard CMake build options apply, such as `CMAKE_BUILD_TYPE`, etc.

## Documentation

The code is highly documented in-source; however, it's still in active development,
so no separate documentation for now. Please refer to in-source docs for the latest
information.

## License

This project is licensed under [MIT License with exceptions][license]. This projects
also conforms to the requirements of `llvm`'s Apache License v2.0 with LLVM
Exceptions and `fmt`'s MIT License with exceptions.

[license]: https://github.com/MunchDev/Firestorm/blob/main/LICENSE
