cmake_minimum_required(VERSION 4.1)

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

add_library(AlengCore STATIC
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
    src/Core/Modules/NativeModule.h
    src/Core/Modules/StdMath.cpp
    src/Core/ModuleManager.cpp
    src/Core/ModuleManager.h
    src/Core/NativeRegistry.cpp
    src/Core/Modules/StdTest.cpp
)

target_include_directories(AlengCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/Core/Generated
)

if (EMSCRIPTEN)
    target_compile_options(AlengCore PRIVATE "-frtti" "-fexceptions")
endif ()

add_dependencies(AlengCore GenerateStdLib)

if (EMSCRIPTEN)
    message(STATUS "Building for WebAssembly (Emscripten)...")

    add_executable(AlengWasm
            src/Apps/Wasm/Bindings.cpp
    )

    target_link_libraries(AlengWasm PRIVATE AlengCore)

    target_compile_options(AlengWasm PRIVATE "-fexceptions" "-frtti")

    target_link_options(AlengWasm PRIVATE
            "-fexceptions"
            "-frtti"
            "--bind"
            "-sWASM=1"
            "-sMODULARIZE=1"
            "-sEXPORT_NAME='createAlengModule'"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sFORCE_FILESYSTEM=1"
            "-sNO_EXIT_RUNTIME=1"
            "-sSTACK_SIZE=5MB"
            "-sDISABLE_EXCEPTION_CATCHING=0"
    )

    set_target_properties(AlengWasm PROPERTIES SUFFIX ".js")

else()
    message(STATUS "Building for Native Terminal (CLI)...")

    add_executable(AlengCLI src/Apps/CLI/main.cpp)
    target_link_libraries(AlengCLI PRIVATE AlengCore)
endif()

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

if (NOT EMSCRIPTEN)
    add_executable(AlengLSP
            src/Apps/LSP/main.cpp
            src/Apps/LSP/LSPTransport.h
            src/Apps/LSP/LSPTransport.cpp
    )

    target_link_libraries(AlengLSP PRIVATE AlengCore nlohmann_json::nlohmann_json)
endif()