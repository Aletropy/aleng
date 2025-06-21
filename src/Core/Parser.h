#pragma once

#include "AST.h"
#include "Lexer.h"

namespace Aleng
{
    class Parser
    {
    public:
        Parser(const std::string &input);
        std::unique_ptr<ProgramNode> ParseProgram();

    private:
        NodePtr Statement();
        NodePtr ParseBlock();
        NodePtr ParseIfStatement();
        NodePtr ParseFunctionDefinition();
        NodePtr Expression();
        NodePtr AddictiveExpression();
        NodePtr Term();
        NodePtr Factor();

        Token Peek();

    private:
        int m_Index = 0;
        std::vector<Token> m_Tokens;
    };
}