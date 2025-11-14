cmake_minimum_required(VERSION 3.16)

project(Aleng LANGUAGES CXX VERSION 0.01)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(STDLIB_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/generate_stdlib.py)
set(STDLIB_INPUT_DIR ${CMAKE_SOURCE_DIR}/stdlib)
set(STDLIB_OUTPUT_HEADER ${CMAKE_CURRENT_BINARY_DIR}/Core/Generated/StdLib.h)

file(GLOB_RECURSE STDLIB_SOURCES "${STDLIB_INPUT_DIR}/*.aleng")

add_custom_command(
        OUTPUT ${STDLIB_OUTPUT_HEADER}
        COMMAND ${Python3_EXECUTABLE} ${STDLIB_SCRIPT} ${STDLIB_INPUT_DIR} ${STDLIB_OUTPUT_HEADER}
        DEPENDS ${STDLIB_SCRIPT} ${STDLIB_SOURCES}
        COMMENT "Generating standard library header from .aleng files..."
)

add_custom_target(
        GenerateStdLib ALL
        DEPENDS ${STDLIB_OUTPUT_HEADER}
)

add_executable(Aleng
    src/main.cpp
    src/Core/Error.h
    src/Core/Error.cpp
    src/Core/AST.h
    src/Core/AST.cpp
    src/Core/Tokens.h
    src/Core/Lexer.h
    src/Core/Lexer.cpp
    src/Core/Parser.h
    src/Core/Parser.cpp
    src/Core/Visitor.h
    src/Core/Visitor.cpp
        src/Modules/NativeModule.h
        src/Modules/StdMath.cpp
        src/Core/ModuleManager.cpp
        src/Core/ModuleManager.h
        src/Core/NativeRegistry.cpp
)

target_include_directories(Aleng PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/Core/Generated
)

add_dependencies(Aleng GenerateStdLib)

