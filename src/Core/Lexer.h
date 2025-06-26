#pragma once

#include <string>
#include <vector>
#include "Tokens.h"

namespace Aleng
{
    class Lexer
    {
    public:
        Lexer(const std::string &input);
        std::vector<Token> Tokenize();

    private:
        Token Next();

    private:
        std::string m_Input;
        int m_Index = 0;
        int m_Line = 1;
        int m_Column = 1;
    };
}