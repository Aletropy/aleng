#pragma once

#include <string>
#include <utility>

#include "SourceRange.h"

namespace Aleng
{
    enum class TokenType
    {
        INTEGER,    // 1 | 500 | -32
        FLOAT,      // 1.25 | 500.0 | -99.5
        STRING,     // "Foo" | "Bar"
        IDENTIFIER, // x | varName | FunName

        LBRACE, // [
        RBRACE, // ]
        LCURLY, // {
        RCURLY, // }
        LPAREN, // (
        RPAREN, // )

        PLUS,          // +
        MINUS,         // -
        MULTIPLY,      // *
        POWER,         // ^
        DIVIDE,        // /
        MODULO,        // %
        GREATER,       // >
        GREATER_EQUAL, // >=
        MINOR,         // <
        MINOR_EQUAL,   // <=
        RANGE,         // ..

        COMMA,     // ,
        DOT,       // .
        COLON,     // :
        SEMICOLON, // ;
        DOLLAR,    // $
        EQUALS,    // ==
        ASSIGN,    // =

        IF,       // If
        ELSE,     // Else
        FOR,      // For
        WHILE,    // While
        IN,       // in
        UNTIL,    // until
        STEP,     // step
        AND,      // and
        OR,       // or
        NOT,      // not
        FUNCTION, // Fn
        END,      // End
        RETURN,   // Return
        BREAK,    // Break
        CONTINUE, // Continue
        IMPORT,   // Import

        TRUE,  // True
        FALSE, // False

        UNKNOWN,
        END_OF_FILE // EOF
    };

    struct Token
    {
        Token(const TokenType type, const char c, SourceRange range)
            : Type(type), Value(std::string(1, c)), Range(std::move(range)) { }
        Token(const TokenType type, std::string str, SourceRange range)
            : Type(type), Value(std::move(str)), Range(std::move(range)) { }

        TokenType Type;
        std::string Value;
        SourceRange Range;
    };

    inline std::string TokenTypeToString(const TokenType type)
    {
        switch (type)
        {
        case TokenType::PLUS:
            return "+";
        case TokenType::MINUS:
            return "-";
        case TokenType::MULTIPLY:
            return "*";
        case TokenType::DIVIDE:
            return "/";
        case TokenType::MODULO:
            return "%";
        case TokenType::AND:
            return "and";
        case TokenType::NOT:
            return "not";
        case TokenType::OR:
            return "or";
        case TokenType::FUNCTION:
            return "Fn";
        case TokenType::INTEGER:
            return "Integer";
        case TokenType::FLOAT:
            return "Float";
        case TokenType::STRING:
            return "String";
        case TokenType::IDENTIFIER:
            return "Identifier";
        case TokenType::ASSIGN:
            return "=";
        case TokenType::LPAREN:
            return "(";
        case TokenType::RPAREN:
            return ")";
        case TokenType::COLON:
            return ":";
        case TokenType::DOT:
                return ".";
        case TokenType::SEMICOLON:
            return ";";
        case TokenType::UNKNOWN:
            return "Unknown";
        case TokenType::LBRACE:
                return "[";
        case TokenType::RBRACE:
                return "]";
        case TokenType::LCURLY:
                return "(";
        case TokenType::RCURLY:
                return ")";
        case TokenType::POWER:
                return "^";
        case TokenType::GREATER:
                return ">";
        case TokenType::GREATER_EQUAL:
                return ">=";
        case TokenType::MINOR:
                return "<";
        case TokenType::MINOR_EQUAL:
                return "<=";
        case TokenType::RANGE:
                return "..";
        case TokenType::COMMA:
                return ",";
        case TokenType::DOLLAR:
                return "$";
        case TokenType::EQUALS:
                return "==";
        case TokenType::IF:
                return "If";
        case TokenType::ELSE:
                return "Else";
        case TokenType::FOR:
                return "For";
        case TokenType::WHILE:
                return "While";
        case TokenType::IN:
                return "in";
        case TokenType::UNTIL:
                return "until";
        case TokenType::STEP:
                return "step";
        case TokenType::END:
                return "End";
        case TokenType::RETURN:
                return "Return";
        case TokenType::BREAK:
                return "Break";
        case TokenType::CONTINUE:
                return "Continue";
        case TokenType::IMPORT:
                return "Import";
        case TokenType::TRUE:
                return "True";
        case TokenType::FALSE:
            return "False";
        case TokenType::END_OF_FILE:
            return "EOF";
        }
        return "?";
    }
}
