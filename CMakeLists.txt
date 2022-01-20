cmake_minimum_required(VERSION 3.21)
project(Firestorm
    VERSION 1.0.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# for fmt::format and stuff...
find_package(fmt 8 REQUIRED)

# LLVM stuff
find_package(LLVM 13 REQUIRED)
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(LLVM_LIBS core orcjit native)

include_directories(include)

add_library(Firestorm
    src/custom_exceptions.cpp
    src/codegen.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    )
target_link_libraries(Firestorm PUBLIC fmt::fmt ${LLVM_LIBS})

add_executable(FirestormMain
    src/main.cpp
    src/frontend.cpp
    )
target_link_libraries(FirestormMain PUBLIC Firestorm)
