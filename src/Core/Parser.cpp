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
        if (token.Type == TokenType::FUNCTION)
        {
            return ParseFunctionDefinition();
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

    NodePtr Parser::ParseFunctionDefinition()
    {
        m_Index++;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected function name");

        auto nameToken = m_Tokens[m_Index];
        std::string funcName = nameToken.Value;
        if (Peek().Type != TokenType::LPAREN)
            throw std::runtime_error("Expected '(' after function name");
        m_Index += 2;
        std::vector<Parameter> params;
        bool expectComma = false;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RPAREN)
        {
            if (expectComma && m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::COMMA && m_Tokens[m_Index].Type != TokenType::RPAREN)
                throw std::runtime_error("Expected ',' between parameters or ')'");

            if (expectComma)
                m_Index++;

            bool isVariadic = false;
            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::DOLLAR)
            {
                m_Index++;
                isVariadic = true;
            }

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
                throw std::runtime_error("Expected parameter name");

            auto paramToken = m_Tokens[m_Index];
            std::string paramName = paramToken.Value;
            std::optional<std::string> typeName;

            m_Index++;

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COLON)
            {
                if (Peek().Type != TokenType::IDENTIFIER)
                    throw std::runtime_error("Expected type name after ':'");
                m_Index++;
                auto typeNameToken = m_Tokens[m_Index];
                typeName = typeNameToken.Value;
                m_Index++;
            }

            params.emplace_back(paramName, typeName, isVariadic);
            if (isVariadic && Peek().Type != TokenType::RPAREN)
                throw std::runtime_error("Variadic parameter '$" + paramName + "' must be the last parameter.");
            expectComma = true;
        }

        m_Index++;
        std::vector<NodePtr> bodyStatements;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            bodyStatements.push_back(Statement());
        }

        NodePtr body = std::make_unique<BlockNode>(std::move(bodyStatements));
        m_Index++;

        return std::make_unique<FunctionDefinitionNode>(funcName, std::move(params), std::move(body));
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
                bool expectCommaArgs = false;

                do
                {
                    if (expectCommaArgs && m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::COMMA)
                        throw std::runtime_error("Expected ',' between function arguments");
                    if (expectCommaArgs)
                        m_Index++;
                    if (m_Tokens[m_Index].Type != TokenType::RPAREN)
                        args.push_back(Expression());
                    expectCommaArgs = true;
                } while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COMMA);

                if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RPAREN)
                    throw std::runtime_error("Expected ')' after function arguments.");

                m_Index++;
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

        if (token.Type == TokenType::MINUS)
        {
            m_Index++;
            NodePtr operand = Factor();
            auto zero = std::make_unique<IntegerNode>(0);
            return std::make_unique<BinaryExpressionNode>(TokenType::MINUS, std::move(zero), std::move(operand));
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