#pragma once

#include "AST.h"
#include "Error.h"
#include "Lexer.h"

namespace Aleng
{
    class Parser
    {
    public:
        explicit Parser(const std::string &input, std::string filepath = "unknown");
        std::unique_ptr<ProgramNode> ParseProgram();

        [[nodiscard]] const std::vector<AlengError>& GetErrors() const { return m_Errors; }
        [[nodiscard]] bool HasErrors() const { return !m_Errors.empty(); }

    private:
        NodePtr Statement();
        NodePtr ParseBlock();
        NodePtr ParseIfStatement();
        NodePtr ParseForStatement();
        NodePtr ParseWhileStatement();
        NodePtr ParseFunctionDefinition();
        NodePtr ParseFunctionLiteral();
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
        void ReportError(const std::string& msg, SourceRange loc);
        void Synchronize();

    private:
        int m_Index = 0;
        std::vector<Token> m_Tokens;
        std::vector<AlengError> m_Errors;
    };
}