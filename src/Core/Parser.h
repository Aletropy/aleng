#pragma once

#include "AST.h"
#include "Lexer.h"

namespace Aleng
{
    class Parser
    {
    public:
        Parser(const std::string &input);
        NodePtr Parse();

    private:
        NodePtr Expression();
        NodePtr Term();
        NodePtr Factor();

        Token Peek();

    private:
        int m_Index = 0;
        std::vector<Token> m_Tokens;
    };
}