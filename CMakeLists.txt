cmake_minimum_required(VERSION 3.16)

project(Aleng LANGUAGES CXX VERSION 0.01)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

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
)

target_include_directories(Aleng PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

