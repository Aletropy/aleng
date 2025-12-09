#include "Lexer.h"

#include <iostream>
#include <utility>
#include "Error.h"

namespace Aleng
{
    Lexer::Lexer(std::string input, std::string  filepath)
        : m_Input(std::move(input)), m_FilePath(std::move(filepath))
    {
        m_Line = 1;
        m_Column = 1;
        m_Index = 0;
    }

    char Lexer::Peek(const int offset) const {
        if (m_Index + offset >= m_Input.length()) return '\0';
        return m_Input[m_Index + offset];
    }

    char Lexer::Advance() {
        if (m_Index >= m_Input.length()) return '\0';
        char c = m_Input[m_Index];
        m_Index++;

        if (c == '\n') {
            m_Line++;
            m_Column = 1;
        }
        else if ((c & 0xC0) == 0x80) {

        }
        else if ((c & 0xF8) == 0xF0) {
            m_Column += 2;
        }
        else {
            m_Column++;
        }
        return c;
    }

    Token Lexer::MakeToken(const TokenType type, std::string value, const SourceLocation start) const
    {
        const SourceLocation end = {m_Line, m_Column};
        return Token(type, std::move(value), SourceRange{start, end, m_FilePath});
    }

    AlengError Lexer::MakeAlengError(const std::string& message, const SourceLocation location) const
    {
        const SourceLocation end = {m_Line, m_Column};
        return AlengError(message, SourceRange{location, end, m_FilePath});
    }

    void Lexer::SkipWhitespace() {
        while (true) {
            if (const char c = Peek(); isspace(c)) {
                Advance();
            } else if (c == '#') {
                if (Peek(1) == '#') {
                    Advance(); Advance();
                    while (Peek() != '\0' && !(Peek() == '#' && Peek(1) == '#')) {
                        Advance();
                    }
                    if (Peek() != '\0') { Advance(); Advance(); }
                } else {
                    while (Peek() != '\0' && Peek() != '\n') Advance();
                }
            } else {
                break;
            }
        }
    }

    Token Lexer::Next()
    {
        SkipWhitespace();

        if (m_Index >= m_Input.length()) {
            return MakeToken(TokenType::END_OF_FILE, "", {m_Line, m_Column});
        }

        const SourceLocation startLoc = {m_Line, m_Column};
        const auto c = Peek();

        // Number verification
        if (std::isdigit(c))
        {
            std::string value;

            while (std::isdigit(Peek())) value += Advance();

            if (Peek() == '.' && std::isdigit(Peek(1)))
            {
                value += Advance();
                while (std::isdigit(Peek())) value += Advance();

                return MakeToken(TokenType::FLOAT, value, startLoc);
            }

            return MakeToken(TokenType::INTEGER, value, startLoc);
        }
        if (std::isalpha(c) || c == '_')
        {
            std::string value;

            while (std::isalnum(Peek()) || Peek() == '_') value += Advance();

            if (value == "If")
                return MakeToken(TokenType::IF, value, startLoc);
            if (value == "Else")
                return MakeToken(TokenType::ELSE, value, startLoc);
            if (value == "While")
                return MakeToken(TokenType::WHILE, value, startLoc);
            if (value == "For")
                return MakeToken(TokenType::FOR, value, startLoc);
            if (value == "Fn")
                return MakeToken(TokenType::FUNCTION, value, startLoc);
            if (value == "Return")
                return MakeToken(TokenType::RETURN, value, startLoc);
            if (value == "Break")
                return MakeToken(TokenType::BREAK, value, startLoc);
            if (value == "Continue")
                return MakeToken(TokenType::CONTINUE, value, startLoc);
            if (value == "Import")
                return MakeToken(TokenType::IMPORT, value, startLoc);
            if (value == "End")
                return MakeToken(TokenType::END, value, startLoc);
            if (value == "True")
                return MakeToken(TokenType::TRUE, value, startLoc);
            if (value == "False")
                return MakeToken(TokenType::FALSE, value, startLoc);
            if (value == "in")
                return MakeToken(TokenType::IN, value, startLoc);
            if (value == "until")
                return MakeToken(TokenType::UNTIL, value, startLoc);
            if (value == "step")
                return MakeToken(TokenType::STEP, value, startLoc);
            if (value == "and")
                return MakeToken(TokenType::AND, value, startLoc);
            if (value == "or")
                return MakeToken(TokenType::OR, value, startLoc);
            if (value == "not")
                return MakeToken(TokenType::NOT, value, startLoc);

            return MakeToken(TokenType::IDENTIFIER, value, startLoc);
        }

        if (c == '$')
        {
            Advance();
            return MakeToken(TokenType::DOLLAR, "$", startLoc);
        }

        if (c == '"')
        {
            Advance();
            std::string value;

            while (Peek() != '"' && Peek() != '\0')
            {
                if (Peek() == '\\')
                {
                    Advance();
                    if (Peek() != '\0') value += Advance();
                }
                else
                    value += Advance();
            }

             if (Peek() != '"') throw MakeAlengError("Unterminated string", startLoc);
            Advance();

            return MakeToken(TokenType::STRING, value, startLoc);
        }

        switch (c) {
            case '+': Advance(); return MakeToken(TokenType::PLUS, "+", startLoc);
            case '-': Advance(); return MakeToken(TokenType::MINUS, "-", startLoc);
            case '*': Advance(); return MakeToken(TokenType::MULTIPLY, "*", startLoc);
            case '/': Advance(); return MakeToken(TokenType::DIVIDE, "/", startLoc);
            case '%': Advance(); return MakeToken(TokenType::MODULO, "%", startLoc);
            case '(': Advance(); return MakeToken(TokenType::LPAREN, "(", startLoc);
            case ')': Advance(); return MakeToken(TokenType::RPAREN, ")", startLoc);
            case '{': Advance(); return MakeToken(TokenType::LCURLY, "{", startLoc);
            case '}': Advance(); return MakeToken(TokenType::RCURLY, "}", startLoc);
            case '[': Advance(); return MakeToken(TokenType::LBRACE, "[", startLoc);
            case ']': Advance(); return MakeToken(TokenType::RBRACE, "]", startLoc);
            case ',': Advance(); return MakeToken(TokenType::COMMA, ",", startLoc);
            case ';': Advance(); return MakeToken(TokenType::SEMICOLON, ";", startLoc);
            case ':': Advance(); return MakeToken(TokenType::COLON, ":", startLoc);
            case '.':
                if (Peek(1) == '.') { Advance(); Advance(); return MakeToken(TokenType::RANGE, "..", startLoc); }
                Advance();
                return MakeToken(TokenType::DOT, ".", startLoc);
            case '=':
                if (Peek(1) == '=') { Advance(); Advance(); return MakeToken(TokenType::EQUALS, "==", startLoc); }
                Advance();
                return MakeToken(TokenType::ASSIGN, "=", startLoc);
            case '!':
                if (Peek(1) == '=') { Advance(); Advance(); return MakeToken(TokenType::EQUALS, "!=", startLoc); }
                Advance();
                break;
            case '>':
                if (Peek(1) == '=') { Advance(); Advance(); return MakeToken(TokenType::GREATER_EQUAL, ">=", startLoc); }
                Advance();
                return MakeToken(TokenType::GREATER, ">", startLoc);
            case '<':
                if (Peek(1) == '=') { Advance(); Advance(); return MakeToken(TokenType::MINOR_EQUAL, "<=", startLoc); }
                Advance();
                return MakeToken(TokenType::MINOR, "<", startLoc);
            default:
                Advance();
                return MakeToken(TokenType::UNKNOWN, std::string(1, c), startLoc);;
        }

        return MakeToken(TokenType::UNKNOWN, std::string(1, c), startLoc);
    }

    std::vector<Token> Lexer::Tokenize()
    {
        auto tokens = std::vector<Token>();
        Token token = Next();
        while (token.Type != TokenType::END_OF_FILE)
        {
            tokens.push_back(token);
            token = Next();
        }
        tokens.push_back(token);
        return tokens;
    }
}
