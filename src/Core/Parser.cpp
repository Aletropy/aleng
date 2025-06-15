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

    NodePtr Parser::Parse()
    {
        return Expression();
    }

    NodePtr Parser::Expression()
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
        return {TokenType::END, ""};
    }
}