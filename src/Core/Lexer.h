#pragma once

#include <string>
#include <vector>

#include "Error.h"
#include "Tokens.h"

namespace Aleng
{
    class Lexer
    {
    public:
        explicit Lexer(std::string input, std::string  filepath = "unknown");
        std::vector<Token> Tokenize();

    private:
        [[nodiscard]] char Peek(int offset = 0) const;
        char Advance();
        [[nodiscard]] Token MakeToken(TokenType type, std::string value, SourceLocation start) const;

        AlengError MakeAlengError(const std::string &message, SourceLocation location) const;

        void SkipWhitespace();

        Token Next();

    private:
        std::string m_Input;
        std::string m_FilePath;
        int m_Index = 0;
        int m_Line = 1;
        int m_Column = 1;
    };
}