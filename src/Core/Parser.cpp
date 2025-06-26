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
            return ParseIfStatement();
        else if (token.Type == TokenType::FUNCTION)
            return ParseFunctionDefinition();
        else if (token.Type == TokenType::FOR)
            return ParseForStatement();

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

        NodePtr thenBranch = std::make_unique<BlockNode>(std::move(thenStatements), m_Tokens[m_Index].Location);
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
            elseBranch = std::make_unique<BlockNode>(std::move(elseStatements), m_Tokens[m_Index].Location);
        }

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::END)
            m_Index++;
        else
            throw std::runtime_error("Expected 'end' to close 'if' statement.");

        return std::make_unique<IfNode>(
            std::move(condition), std::move(thenBranch), std::move(elseBranch), m_Tokens[m_Index].Location);
    }

    NodePtr Parser::ParseForStatement()
    {
        m_Index++;

        if (m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected iterator variable name after 'For'.");

        std::string iteratorVarName = m_Tokens[m_Index].Value;
        m_Index++;

        if (m_Index >= m_Tokens.size())
            throw std::runtime_error("Unexpected end of input after For <iterator>.");

        NodePtr body;
        std::vector<NodePtr> bodyStatements;

        if (m_Tokens[m_Index].Type == TokenType::ASSIGN)
        {
            m_Index++;

            NodePtr startExpr = Expression();
            bool isUntil = false;
            NodePtr endExpr;
            NodePtr stepExpr = nullptr;

            if (m_Tokens[m_Index].Type == TokenType::RANGE)
            {
                m_Index++;
                isUntil = false;
            }
            else if (m_Tokens[m_Index].Type == TokenType::UNTIL)
            {
                m_Index++;
                isUntil = true;
            }
            else
            {
                throw std::runtime_error("Expected '..' or 'until' in For loop range.");
            }

            endExpr = Expression();

            if (m_Tokens[m_Index].Type == TokenType::STEP)
            {
                m_Index++;
                stepExpr = Expression();
            }

            while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END)
                bodyStatements.push_back(Statement());
            if (m_Tokens[m_Index].Type != TokenType::END)
                throw std::runtime_error("Expected 'End' to close 'For' statement.");

            m_Index++;
            body = std::make_unique<BlockNode>(std::move(bodyStatements), m_Tokens[m_Index].Location);

            ForNumericRange numericInfo = {iteratorVarName, std::move(startExpr), std::move(endExpr), std::move(stepExpr), isUntil};
            return std::make_unique<ForStatementNode>(numericInfo, std::move(body), m_Tokens[m_Index].Location);
        }
        else if (m_Tokens[m_Index].Type == TokenType::IN)
        {
            m_Index++;
            NodePtr collectionExpr = Expression();

            while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END)
                bodyStatements.push_back(Statement());
            if (m_Tokens[m_Index].Type != TokenType::END)
                throw std::runtime_error("Expected 'End' to close 'For' statement.");

            body = std::make_unique<BlockNode>(std::move(bodyStatements), m_Tokens[m_Index].Location);
            m_Index++;

            ForCollectionRange collectionInfo = {iteratorVarName, std::move(collectionExpr)};
            return std::make_unique<ForStatementNode>(collectionInfo, std::move(body), m_Tokens[m_Index].Location);
        }
        else
            throw std::runtime_error("Expected '=' or 'in' after iterator variable in For loop.");
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

        NodePtr body = std::make_unique<BlockNode>(std::move(bodyStatements), m_Tokens[m_Index].Location);
        m_Index++;

        return std::make_unique<FunctionDefinitionNode>(funcName, std::move(params), std::move(body), m_Tokens[m_Index].Location);
    }

    NodePtr Parser::ParseBlock()
    {
        std::vector<NodePtr> statements;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
            statements.push_back(Statement());

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::END)
            m_Index++;

        return std::make_unique<BlockNode>(std::move(statements), m_Tokens[m_Index].Location);
    }

    NodePtr Parser::ParseListLiteral()
    {
        m_Index++;
        std::vector<NodePtr> elements;
        bool expectComma = false;

        if (m_Tokens[m_Index].Type != TokenType::RBRACE)
        {
            do
            {
                if (expectComma)
                {
                    if (m_Tokens[m_Index].Type != TokenType::COMMA)
                        throw std::runtime_error("Expected ',' or ']' in list literal.");
                    m_Index++;
                }
                elements.push_back(Expression());
                expectComma = true;
            } while (m_Tokens[m_Index].Type == TokenType::COMMA);
        }

        if (m_Tokens[m_Index].Type != TokenType::RBRACE)
            throw std::runtime_error("Expected ']' to close list literal");
        m_Index++;
        return std::make_unique<ListNode>(std::move(elements), m_Tokens[m_Index].Location);
    }

    NodePtr Parser::Expression()
    {
        auto left = AddictiveExpression();

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::ASSIGN)
        {
            if (!dynamic_cast<IdentifierNode *>(left.get()) && !dynamic_cast<ListAccessNode *>(left.get()))
                throw std::runtime_error("Invalid left-hand side in assignment expression.");

            m_Index++;
            NodePtr right = Statement();
            return std::make_unique<AssignExpressionNode>(
                std::move(left), std::move(right), m_Tokens[m_Index].Location);
        }

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::EQUALS))
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = AddictiveExpression();
            left = std::make_unique<EqualsExpressionNode>(std::move(left), std::move(right), op.Value == "!=", m_Tokens[m_Index].Location);
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
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), m_Tokens[m_Index].Location);
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
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), m_Tokens[m_Index].Location);
        }

        return left;
    }

    NodePtr Parser::Factor()
    {
        auto token = m_Tokens[m_Index];
        NodePtr primaryExpr = nullptr;

        if (token.Type == TokenType::TRUE)
        {
            m_Index++;
            primaryExpr = std::make_unique<BooleanNode>(true, token.Location);
        }
        else if (token.Type == TokenType::FALSE)
        {
            m_Index++;
            primaryExpr = std::make_unique<BooleanNode>(false, token.Location);
        }
        else if (token.Type == TokenType::INTEGER)
        {
            m_Index++;
            primaryExpr = std::make_unique<IntegerNode>(std::stoll(token.Value), token.Location);
        }
        else if (token.Type == TokenType::FLOAT)
        {
            m_Index++;
            primaryExpr = std::make_unique<FloatNode>(std::stof(token.Value), token.Location);
        }

        else if (token.Type == TokenType::STRING)
        {
            m_Index++;
            primaryExpr = std::make_unique<StringNode>(token.Value, token.Location);
        }
        else if (token.Type == TokenType::LBRACE)
            primaryExpr = ParseListLiteral();
        else if (token.Type == TokenType::LPAREN)
        {
            m_Index++;
            auto expr = Expression();
            if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::RPAREN)
                throw std::runtime_error("Expected ')'");
            m_Index++;
            primaryExpr = std::move(expr);
        }
        else if (token.Type == TokenType::MINUS)
        {
            m_Index++;
            NodePtr operand = Factor();
            auto zero = std::make_unique<IntegerNode>(0, token.Location);
            primaryExpr = std::make_unique<BinaryExpressionNode>(TokenType::MINUS, std::move(zero), std::move(operand), token.Location);
        }
        else if (token.Type == TokenType::IDENTIFIER)
        {
            m_Index++;
            primaryExpr = std::make_unique<IdentifierNode>(token.Value, token.Location);
        }
        else if (token.Type == TokenType::MODULE)
        {
            m_Index++;
            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::STRING)
            {
                auto pathStr = m_Tokens[m_Index++].Value;
                return std::make_unique<ImportModuleNode>(pathStr, token.Location);
            }
            throw std::runtime_error("Expected module name after 'module' keyword.");
        }
        else
            throw std::runtime_error("Unexpected token: " + token.Value);

        while (m_Index < m_Tokens.size())
        {
            if (m_Tokens[m_Index].Type == TokenType::LPAREN)
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

                primaryExpr = std::make_unique<FunctionCallNode>(std::move(primaryExpr), std::move(args), token.Location);
            }
            else if (m_Tokens[m_Index].Type == TokenType::LBRACE)
            {
                m_Index++;
                auto indexExpr = Expression();
                if (m_Tokens[m_Index].Type != TokenType::RBRACE)
                    throw std::runtime_error("Expected ']' after index expression.");
                m_Index++;
                primaryExpr = std::make_unique<ListAccessNode>(std::move(primaryExpr), std::move(indexExpr), token.Location);
            }
            else
                break;
        }

        return primaryExpr;
    }

    Token Parser::Peek()
    {
        if (m_Index + 1 < m_Tokens.size())
            return m_Tokens[m_Index + 1];
        return {TokenType::END_OF_FILE, "", m_Tokens[m_Index].Location};
    }
}