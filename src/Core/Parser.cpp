#include "AST.h"
#include "Parser.h"

namespace Aleng
{
    Parser::Parser(const std::string &input)
    {
        auto lexer = Lexer(input);
        m_Tokens = lexer.Tokenize();
        m_Index = 0;
    }

    std::unique_ptr<ProgramNode> Parser::ParseProgram()
    {
        auto program = std::make_unique<ProgramNode>();

        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            program->Statements.push_back(Statement());
        }

        return program;
    }

    NodePtr Parser::Statement()
    {
        auto token = m_Tokens[m_Index];

        if (token.Type == TokenType::IF)
        {
            return ParseIfStatement();
        }

        auto expr = Expression();
        return expr;
    }

    NodePtr Parser::ParseIfStatement()
    {
        m_Index++;
        auto condition = Expression();

        std::vector<NodePtr> thenStatements;

        while (m_Index < m_Tokens.size() &&
               m_Tokens[m_Index].Type != TokenType::ELSE &&
               m_Tokens[m_Index].Type != TokenType::END &&
               m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            thenStatements.push_back(Statement());
        }

        NodePtr thenBranch = std::make_unique<BlockNode>(std::move(thenStatements));
        NodePtr elseBranch = nullptr;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::ELSE)
        {
            m_Index++;
            std::vector<NodePtr> elseStatements;
            while (m_Index < m_Tokens.size() &&
                   m_Tokens[m_Index].Type != TokenType::END &&
                   m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
            {
                elseStatements.push_back(Statement());
            }
            elseBranch = std::make_unique<BlockNode>(std::move(elseStatements));
        }

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::END)
            m_Index++;
        else
            throw std::runtime_error("Expected 'end' to close 'if' statement.");

        return std::make_unique<IfNode>(
            std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }

    NodePtr Parser::ParseBlock()
    {
        std::vector<NodePtr> statements;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
            statements.push_back(Statement());

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::END)
            m_Index++;

        return std::make_unique<BlockNode>(std::move(statements));
    }

    NodePtr Parser::Expression()
    {
        auto left = AddictiveExpression();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::EQUALS))
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = AddictiveExpression();
            left = std::make_unique<EqualsExpressionNode>(std::move(left), std::move(right), op.Value == "!=");
        }

        return left;
    }

    NodePtr Parser::AddictiveExpression()
    {
        auto left = Term();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::PLUS || m_Tokens[m_Index].Type == TokenType::MINUS))
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = Term();
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right));
        }

        return left;
    }

    NodePtr Parser::Term()
    {
        auto left = Factor();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::MULTIPLY || m_Tokens[m_Index].Type == TokenType::DIVIDE))
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = Factor();
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right));
        }

        return left;
    }

    NodePtr Parser::Factor()
    {
        auto token = m_Tokens[m_Index];

        if (token.Type == TokenType::INTEGER)
        {
            m_Index++;
            return std::make_unique<IntegerNode>(std::stoi(token.Value));
        }
        if (token.Type == TokenType::FLOAT)
        {
            m_Index++;
            return std::make_unique<FloatNode>(std::stof(token.Value));
        }

        if (token.Type == TokenType::STRING)
        {
            m_Index++;
            return std::make_unique<StringNode>(token.Value);
        }

        if (token.Type == TokenType::IDENTIFIER)
        {
            m_Index++;
            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::ASSIGN)
            {
                m_Index++;
                auto right = Expression();
                return std::make_unique<AssignExpressionNode>(token.Value, std::move(right));
            }
            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::LPAREN)
            {
                m_Index++;
                std::vector<NodePtr> args;

                if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RPAREN)
                {
                    args.push_back(Expression());

                    while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COMMA)
                    {
                        m_Index++;
                        args.push_back(Expression());
                    }
                }

                if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::RPAREN)
                {
                    m_Index++;
                }
                else
                {
                    throw std::runtime_error("Expected ')' after function arguments.");
                }

                return std::make_unique<FunctionCallNode>(token.Value, std::move(args));
            }

            return std::make_unique<IdentifierNode>(token.Value);
        }

        if (token.Type == TokenType::MODULE)
        {
            m_Index++;

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::STRING)
            {
                auto pathStr = m_Tokens[m_Index++].Value;
                return std::make_unique<ImportModuleNode>(pathStr);
            }

            throw std::runtime_error("Expected module name after 'module' keyword.");
        }

        if (token.Type == TokenType::LPAREN)
        {
            m_Index++;
            auto expr = Expression();
            if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::RPAREN)
                throw std::runtime_error("Expected ')'");
            m_Index++;
            return expr;
        }

        throw std::runtime_error("Unexpected token: " + token.Value);
    }

    Token Parser::Peek()
    {
        if (m_Index + 1 < m_Tokens.size())
            return m_Tokens[m_Index + 1];
        return {TokenType::END_OF_FILE, ""};
    }
}