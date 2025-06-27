#pragma once

#include <string>

namespace Aleng
{
    enum class TokenType
    {
        INTEGER,
        FLOAT,
        STRING,
        IDENTIFIER,

        LBRACE, // [
        RBRACE, // ]
        LCURLY, // {
        RCURLY, // }
        LPAREN, // (
        RPAREN, // )

        PLUS,
        MINUS,
        MULTIPLY,
        POWER,
        DIVIDE,
        GREATER,
        MINOR,
        RANGE, // ..

        COMMA,
        COLON,
        SEMICOLON,
        DOLLAR,
        EQUALS,
        ASSIGN,

        IF,
        ELSE,
        FOR,
        WHILE,
        IN,
        UNTIL,
        STEP,
        FUNCTION,
        END,
        RETURN,
        BREAK,
        CONTINUE,
        MODULE,

        TRUE,
        FALSE,

        UNKNOWN,
        END_OF_FILE
    };

    struct TokenLocation
    {
        int Line = 1;
        int Column = 1;
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
        case TokenType::SEMICOLON:
            return ";";
        case TokenType::UNKNOWN:
            return "Unknown";
        default:
            return "InvalidToken";
        }
    }
}