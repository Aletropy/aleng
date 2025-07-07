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
        NodePtr ParseForStatement();
        NodePtr ParseWhileStatement();
        NodePtr ParseFunctionDefinition();
        NodePtr ParseListLiteral();
        NodePtr ParseMapLiteral();

        NodePtr Expression();
        NodePtr LogicalOrExpression();
        NodePtr LogicalAndExpression();
        NodePtr EqualityExpression();
        NodePtr ComparisonExpression();
        NodePtr AddictiveExpression();
        NodePtr Term();
        NodePtr UnaryExpression();
        NodePtr Factor();

        Token Peek();

    private:
        int m_Index = 0;
        std::vector<Token> m_Tokens;
    };
}