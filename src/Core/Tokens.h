#pragma once

#include <string>

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

    struct TokenLocation
    {
        int Line = 1;
        int Column = 1;
        std::string FilePath;
    };

    struct Token
    {
        Token(TokenType type, char c, TokenLocation loc)
        {
            Type = type;
            Value = c;
            Location = loc;
        }
        Token(TokenType type, const std::string &str, TokenLocation loc)
        {
            Type = type;
            Value = str;
            Location = loc;
        }

        TokenType Type;
        std::string Value;
        TokenLocation Location;
    };

    inline std::string TokenTypeToString(TokenType type)
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
        default:
            return "InvalidToken";
        }
    }
}