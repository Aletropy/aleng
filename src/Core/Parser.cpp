#include "AST.h"
#include "Parser.h"

#include <iostream>

#include "Error.h"

// Internal exception used only for control flow during parsing errors
struct ParserSyncException final : std::exception{};

namespace Aleng
{
    Parser::Parser(const std::string &input, std::string filepath)
    {
        auto lexer = Lexer(input, std::move(filepath));
        m_Tokens = lexer.Tokenize();
        m_Index = 0;
    }

    std::unique_ptr<ProgramNode> Parser::ParseProgram()
    {
        auto program = std::make_unique<ProgramNode>();

        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            try
            {
                if (NodePtr stmt = Statement())
                    program->Statements.push_back(std::move(stmt));
            }
            catch (const ParserSyncException&)
            {
                Synchronize();
            }
        }

        return program;
    }

    NodePtr Parser::Statement()
    {
        if (m_Index >= m_Tokens.size()) return nullptr; // Safety check

        auto token = m_Tokens[m_Index];

        if (token.Type == TokenType::IF)
            return ParseIfStatement();
        else if (token.Type == TokenType::FUNCTION)
            return ParseFunctionDefinition();
        else if (token.Type == TokenType::FOR)
            return ParseForStatement();
        else if (token.Type == TokenType::WHILE)
            return ParseWhileStatement();
        else if (token.Type == TokenType::RETURN)
        {
            m_Index++;
            NodePtr returnValue = nullptr;

            if (m_Index < m_Tokens.size() &&
                m_Tokens[m_Index].Type != TokenType::END &&
                m_Tokens[m_Index].Type != TokenType::ELSE &&
                m_Tokens[m_Index].Type != TokenType::SEMICOLON)
            {
                 returnValue = Expression();
            }
            return std::make_unique<ReturnNode>(std::move(returnValue), token.Range);
        }
        else if (token.Type == TokenType::BREAK)
        {
            m_Index++;
            return std::make_unique<BreakNode>(token.Range);
        }
        else if (token.Type == TokenType::CONTINUE)
        {
            m_Index++;
            return std::make_unique<ContinueNode>(token.Range);
        }

        auto expr = Expression();
        return expr;
    }

    NodePtr Parser::ParseIfStatement()
    {
        Token startToken = m_Tokens[m_Index];
        m_Index++;

        auto condition = Expression();

        if (m_Index >= m_Tokens.size())
        {
             ReportError("Unexpected end of file inside 'If' condition.", startToken.Range);
             throw ParserSyncException();
        }

        auto thenBlockStartLoc = m_Tokens[m_Index].Range;
        std::vector<NodePtr> thenStatements;

        while (m_Index < m_Tokens.size() &&
               m_Tokens[m_Index].Type != TokenType::ELSE &&
               m_Tokens[m_Index].Type != TokenType::END &&
               m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            try {
                if (auto stmt = Statement())
                    thenStatements.push_back(std::move(stmt));
            } catch (const ParserSyncException&) {
                Synchronize();
            }
        }

        NodePtr thenBranch = std::make_unique<BlockNode>(std::move(thenStatements), thenBlockStartLoc);
        NodePtr elseBranch = nullptr;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::ELSE)
        {
            m_Index++;
            auto elseBlockStartLoc = m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : thenBlockStartLoc;
            std::vector<NodePtr> elseStatements;
            while (m_Index < m_Tokens.size() &&
                   m_Tokens[m_Index].Type != TokenType::END &&
                   m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
            {
                try {
                    if (auto stmt = Statement())
                        elseStatements.push_back(std::move(stmt));
                } catch (const ParserSyncException&) {
                    Synchronize();
                }
            }
            elseBranch = std::make_unique<BlockNode>(std::move(elseStatements), elseBlockStartLoc);
        }

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::END)
            m_Index++;
        else
        {
            ReportError("Expected 'End' keyword to close 'If' statement.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
            throw ParserSyncException();
        }

        Token endToken = m_Tokens[m_Index - 1];
        SourceRange fullRange;
        fullRange.Start = startToken.Range.Start;
        fullRange.End = endToken.Range.End;
        fullRange.FilePath = startToken.Range.FilePath;

        return std::make_unique<IfNode>(
            std::move(condition), std::move(thenBranch), std::move(elseBranch), fullRange);
    }

    NodePtr Parser::ParseForStatement()
    {
        Token startToken = m_Tokens[m_Index];
        m_Index++;

        if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
        {
            ReportError("Expected iterator variable name after 'For'.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
            throw ParserSyncException();
        }

        std::string iteratorVarName = m_Tokens[m_Index].Value;
        m_Index++;

        if (m_Index >= m_Tokens.size())
        {
            ReportError("Unexpected end of input after For <iterator>.", m_Tokens[m_Index - 1].Range);
            throw ParserSyncException();
        }

        NodePtr body;
        auto bodyStartLoc = m_Tokens[m_Index].Range;
        std::vector<NodePtr> bodyStatements;

        if (m_Tokens[m_Index].Type == TokenType::ASSIGN)
        {
            m_Index++;

            NodePtr startExpr = Expression();
            bool isUntil = false;
            NodePtr endExpr;
            NodePtr stepExpr = nullptr;

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::RANGE)
            {
                m_Index++;
                isUntil = false;
            }
            else if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::UNTIL)
            {
                m_Index++;
                isUntil = true;
            }
            else
            {
                ReportError("Expected '..' or 'until' in numeric For loop range.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : bodyStartLoc);
                throw ParserSyncException();
            }

            endExpr = Expression();

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::STEP)
            {
                m_Index++;
                stepExpr = Expression();
            }

            while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END)
            {
                try {
                    if (auto stmt = Statement())
                        bodyStatements.push_back(std::move(stmt));
                } catch (const ParserSyncException&) {
                    Synchronize();
                }
            }

            if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::END)
            {
                ReportError("Expected 'End' to close 'For' statement.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
                throw ParserSyncException();
            }

            m_Index++;
            body = std::make_unique<BlockNode>(std::move(bodyStatements), bodyStartLoc);
            ForNumericRange numericInfo = {iteratorVarName, std::move(startExpr), std::move(endExpr), std::move(stepExpr), isUntil};

            Token endToken = m_Tokens[m_Index - 1];
            SourceRange fullRange;
            fullRange.Start = startToken.Range.Start;
            fullRange.End = endToken.Range.End;
            fullRange.FilePath = startToken.Range.FilePath;

            return std::make_unique<ForStatementNode>(numericInfo, std::move(body), fullRange);
        }
        else if (m_Tokens[m_Index].Type == TokenType::IN)
        {
            m_Index++;
            NodePtr collectionExpr = Expression();

            while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END)
            {
                try {
                    if (auto stmt = Statement())
                        bodyStatements.push_back(std::move(stmt));
                } catch (const ParserSyncException&) {
                    Synchronize();
                }
            }

            if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::END)
            {
                ReportError("Expected 'End' to close 'For' statement.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
                throw ParserSyncException();
            }

            body = std::make_unique<BlockNode>(std::move(bodyStatements), bodyStartLoc);
            m_Index++;

            ForCollectionRange collectionInfo = {iteratorVarName, std::move(collectionExpr)};
            return std::make_unique<ForStatementNode>(collectionInfo, std::move(body), startToken.Range);
        }

        ReportError("Expected '=' (for range) or 'in' (for collection) after iterator variable in For loop.", m_Tokens[m_Index].Range);
        throw ParserSyncException();
    }

    NodePtr Parser::ParseWhileStatement()
    {
        Token startToken = m_Tokens[m_Index];
        m_Index++;

        if (m_Index >= m_Tokens.size())
        {
            ReportError("Unexpected end of input after While keyword.", startToken.Range);
            throw ParserSyncException();
        }

        NodePtr condition = Expression();

        if (m_Index >= m_Tokens.size())
        {
             ReportError("Unexpected end of input in While loop.", startToken.Range);
             throw ParserSyncException();
        }

        auto bodyStartToken = m_Tokens[m_Index];
        std::vector<NodePtr> bodyStatements;

        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END)
        {
            try {
                if (auto stmt = Statement())
                    bodyStatements.push_back(std::move(stmt));
            } catch (const ParserSyncException&) {
                Synchronize();
            }
        }

        if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::END)
        {
            ReportError("Expected 'End' to close 'While' statement.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
            throw ParserSyncException();
        }

        m_Index++;
        NodePtr body = std::make_unique<BlockNode>(std::move(bodyStatements), bodyStartToken.Range);

        return std::make_unique<WhileStatementNode>(std::move(condition), std::move(body), startToken.Range);
    }

    NodePtr Parser::ParseFunctionDefinition()
    {
        if (m_Index+1 < m_Tokens.size() && m_Tokens[m_Index+1].Type != TokenType::IDENTIFIER)
            return ParseFunctionLiteral();

        Token startToken = m_Tokens[m_Index];
        m_Index++;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
        {
            ReportError("Expected function name after 'Fn'.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
            throw ParserSyncException();
        }

        std::string funcName = m_Tokens[m_Index].Value;

        m_Index++;
        if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::LPAREN)
        {
            ReportError("Expected '(' after function name.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
            throw ParserSyncException();
        }
        m_Index ++;

        std::vector<Parameter> params;
        bool expectComma = false;
        bool processedVariadic = false;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RPAREN)
        {
            if (processedVariadic)
            {
                ReportError("Variadic parameters must be the last parameters in a function definition.", m_Tokens[m_Index].Range);
                throw ParserSyncException();
            }

            if (expectComma)
            {
                if (m_Tokens[m_Index].Type != TokenType::COMMA && m_Tokens[m_Index].Type != TokenType::RPAREN)
                {
                    ReportError("Expected ',' between parameters or ')' to close parameter list.", m_Tokens[m_Index].Range);
                    throw ParserSyncException();
                }
                if (m_Tokens[m_Index].Type == TokenType::COMMA)
                    m_Index++;
            }

            if (m_Index >= m_Tokens.size()) break; // Safety

            bool isVariadic = false;
            if (m_Tokens[m_Index].Type == TokenType::DOLLAR)
            {
                m_Index++;
                isVariadic = true;
                processedVariadic = true;
            }

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
            {
                ReportError("Expected parameter name.", m_Tokens[m_Index].Range);
                throw ParserSyncException();
            }

            auto paramToken = m_Tokens[m_Index];
            std::string paramName = paramToken.Value;
            std::optional<std::string> typeName;

            m_Index++;

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COLON)
            {
                m_Index++; // Consume ':'
                if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
                {
                    ReportError("Expected type name after ':'.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : paramToken.Range);
                    throw ParserSyncException();
                }

                typeName = m_Tokens[m_Index].Value;
                m_Index++;
            }

            params.emplace_back(paramName, typeName, isVariadic);
            expectComma = true;
        }

        if (m_Index >= m_Tokens.size()) {
             ReportError("Unexpected end of input in function definition.", startToken.Range);
             throw ParserSyncException();
        }
        m_Index++; // Consume ')'

        auto bodyStartLoc = m_Tokens[m_Index].Range;
        std::vector<NodePtr> bodyStatements;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            try {
                if (auto stmt = Statement())
                    bodyStatements.push_back(std::move(stmt));
            } catch (const ParserSyncException&) {
                Synchronize();
            }
        }

        if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::END)
        {
             ReportError("Expected 'End' to close function definition.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
             throw ParserSyncException();
        }

        NodePtr body = std::make_unique<BlockNode>(std::move(bodyStatements), bodyStartLoc);
        m_Index++;

        return std::make_unique<FunctionDefinitionNode>(std::make_optional(funcName), std::move(params), std::move(body), startToken.Range);
    }

    NodePtr Parser::ParseFunctionLiteral()
    {
        const Token startToken = m_Tokens[m_Index];
        m_Index++;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::LPAREN)
        {
            ReportError("Expected '(' for anonymous function declaration or 'name' for default function declaration.", startToken.Range);
            throw ParserSyncException();
        }
        m_Index++;

        std::vector<Parameter> params;
        bool expectComma = false;
        bool processedVariadic = false;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RPAREN)
        {
            // Parameter logic from ParseFunctionDefinition
             if (processedVariadic)
            {
                ReportError("Variadic parameters must be the last parameters.", m_Tokens[m_Index].Range);
                throw ParserSyncException();
            }

            if (expectComma)
            {
                if (m_Tokens[m_Index].Type != TokenType::COMMA && m_Tokens[m_Index].Type != TokenType::RPAREN)
                {
                    ReportError("Expected ',' or ')'", m_Tokens[m_Index].Range);
                    throw ParserSyncException();
                }
                if(m_Tokens[m_Index].Type == TokenType::COMMA) m_Index++;
            }

            if (expectComma)
                m_Index++;

            bool isVariadic = false;
            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::DOLLAR)
            {
                m_Index++;
                isVariadic = true;
                processedVariadic = true;
            }

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
            {
                ReportError("Expected parameter name.", m_Tokens[m_Index].Range);
                throw ParserSyncException();
            }

            auto paramToken = m_Tokens[m_Index];
            std::string paramName = paramToken.Value;
            std::optional<std::string> typeName;

            m_Index++;

            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COLON)
            {
                m_Index++;
                if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
                {
                    ReportError("Expected type name after ':'.", m_Tokens[m_Index].Range);
                    throw ParserSyncException();
                }
                typeName = m_Tokens[m_Index].Value;
                m_Index++;
            }

            params.emplace_back(paramName, typeName, isVariadic);
            expectComma = true;
        }

        if (m_Index >= m_Tokens.size()) {
             ReportError("Unexpected end of input.", startToken.Range);
             throw ParserSyncException();
        }
        m_Index++; // Consume ')'

        auto bodyStartLoc = m_Tokens[m_Index].Range;
        std::vector<NodePtr> bodyStatements;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END)
        {
            try {
                if (auto stmt = Statement())
                    bodyStatements.push_back(std::move(stmt));
            } catch (const ParserSyncException&) {
                Synchronize();
            }
        }

        if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::END)
        {
            ReportError("Expected 'End' to close anonymous function.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
            throw ParserSyncException();
        }

        NodePtr body = std::make_unique<BlockNode>(std::move(bodyStatements), bodyStartLoc);
        m_Index++;

        return std::make_unique<FunctionDefinitionNode>(std::nullopt, std::move(params), std::move(body), startToken.Range);
    }

    NodePtr Parser::ParseBlock()
    {
        std::vector<NodePtr> statements;
        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
             try {
                if (auto stmt = Statement())
                    statements.push_back(std::move(stmt));
            } catch (const ParserSyncException&) {
                Synchronize();
            }
        }

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::END)
            m_Index++;

        return std::make_unique<BlockNode>(std::move(statements), m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : SourceRange{});
    }

    NodePtr Parser::ParseListLiteral()
    {
        m_Index++;
        std::vector<NodePtr> elements;
        bool expectComma = false;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RBRACE)
        {
            do
            {
                if (expectComma)
                {
                    if (m_Tokens[m_Index].Type != TokenType::COMMA)
                    {
                        ReportError("Expected ',' or ']' in list literal.", m_Tokens[m_Index].Range);
                        throw ParserSyncException();
                    }
                    m_Index++;
                }
                elements.push_back(Expression());
                expectComma = true;
            } while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COMMA);
        }

        if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::RBRACE)
        {
            ReportError("Expected ']' to close list literal.", m_Tokens[m_Index-1].Range);
            throw ParserSyncException();
        }
        m_Index++;
        return std::make_unique<ListNode>(std::move(elements), m_Tokens[m_Index-1].Range);
    }

    NodePtr Parser::ParseMapLiteral()
    {
        Token startToken = m_Tokens[m_Index];
        m_Index++;

        std::vector<std::pair<NodePtr, NodePtr>> elements;
        bool expectComma = false;

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RCURLY)
        {
            do
            {
                if (expectComma)
                {
                    if (m_Tokens[m_Index].Type != TokenType::COMMA)
                    {
                        ReportError("Expected ',' or '}' in map literal.", m_Tokens[m_Index].Range);
                        throw ParserSyncException();
                    }
                    m_Index++;
                }

                NodePtr keyExpr = Expression();

                if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::COLON)
                {
                    ReportError("Expected ':' to assign value to key in map literal.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
                    throw ParserSyncException();
                }

                m_Index++;

                NodePtr valueExpr = Expression();

                elements.emplace_back(std::move(keyExpr), std::move(valueExpr));
                expectComma = true;
            } while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COMMA);

            if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::RCURLY)
            {
                ReportError("Expected '}' to close map literal.", m_Index < m_Tokens.size() ? m_Tokens[m_Index].Range : startToken.Range);
                throw ParserSyncException();
            }
        }
        m_Index++;
        return std::make_unique<MapNode>(std::move(elements), startToken.Range);
    }

    NodePtr Parser::Expression()
    {
        auto left = LogicalOrExpression();

        if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::ASSIGN)
        {
            if (
                !dynamic_cast<IdentifierNode *>(left.get()) && !dynamic_cast<ListAccessNode *>(left.get()) &&
                !dynamic_cast<MemberAccessNode*>(left.get()))
            {
                ReportError("Invalid left-hand side in assignment expression.", m_Tokens[m_Index].Range);
                throw ParserSyncException();
            }

            SourceRange assignLocation = m_Tokens[m_Index].Range;

            m_Index++;

            NodePtr right = Expression();
            return std::make_unique<AssignExpressionNode>(
                std::move(left), std::move(right), assignLocation);
        }

        return left;
    }

    NodePtr Parser::LogicalOrExpression()
    {
        auto left = LogicalAndExpression();

        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::OR)
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = LogicalAndExpression();
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), m_Tokens[m_Index].Range);
        }

        return left;
    }

    NodePtr Parser::LogicalAndExpression()
    {
        auto left = EqualityExpression();

        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::AND)
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = EqualityExpression();
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), m_Tokens[m_Index].Range);
        }

        return left;
    }

    NodePtr Parser::EqualityExpression()
    {
        auto left = ComparisonExpression();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::EQUALS))
        {
            auto op = m_Tokens[m_Index++];
            auto right = ComparisonExpression();

            SourceRange range;
            range.Start = left->Location.Start;
            range.End = right->Location.End;
            range.FilePath = left->Location.FilePath;

            left = std::make_unique<EqualsExpressionNode>(std::move(left), std::move(right), op.Value == "!=", range);
        }

        return left;
    }

    NodePtr Parser::ComparisonExpression()
    {
        auto left = AddictiveExpression();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::GREATER || m_Tokens[m_Index].Type == TokenType::GREATER_EQUAL ||
                                             m_Tokens[m_Index].Type == TokenType::MINOR || m_Tokens[m_Index].Type == TokenType::MINOR_EQUAL))
        {
            auto op = m_Tokens[m_Index++];
            auto right = AddictiveExpression();

            SourceRange range;
            range.Start = left->Location.Start;
            range.End = right->Location.End;
            range.FilePath = left->Location.FilePath;

            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), range);
        }

        return left;
    }

    NodePtr Parser::AddictiveExpression()
    {
        auto left = Term();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::PLUS || m_Tokens[m_Index].Type == TokenType::MINUS))
        {
            auto op = m_Tokens[m_Index++];
            auto right = Term();

            SourceRange range;
            range.Start = left->Location.Start;
            range.End = right->Location.End;
            range.FilePath = left->Location.FilePath;

            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), range);
        }

        return left;
    }

    NodePtr Parser::Term()
    {
        auto left = UnaryExpression();

        while (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::MULTIPLY || m_Tokens[m_Index].Type == TokenType::DIVIDE ||
            m_Tokens[m_Index].Type == TokenType::MODULO))
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto right = UnaryExpression();
            left = std::make_unique<BinaryExpressionNode>(op.Type, std::move(left), std::move(right), m_Tokens[m_Index].Range);
        }

        return left;
    }

    NodePtr Parser::UnaryExpression()
    {
        if (m_Index < m_Tokens.size() && (m_Tokens[m_Index].Type == TokenType::NOT || m_Tokens[m_Index].Type == TokenType::MINUS))
        {
            auto op = m_Tokens[m_Index];
            m_Index++;
            auto operand = UnaryExpression();
            if (op.Type == TokenType::MINUS)
            {
                return std::make_unique<BinaryExpressionNode>(op.Type, std::make_unique<IntegerNode>(0, op.Range), std::move(operand), op.Range);
            }
            else
            {
                return std::make_unique<UnaryExpressionNode>(op.Type, std::move(operand), op.Range);
            }
        }

        return Factor();
    }

    NodePtr Parser::Factor()
    {
        if (m_Index >= m_Tokens.size()) {
             ReportError("Unexpected end of expression.", m_Index > 0 ? m_Tokens[m_Index-1].Range : SourceRange{});
             throw ParserSyncException();
        }

        auto token = m_Tokens[m_Index];
        NodePtr primaryExpr = nullptr;

        if (token.Type == TokenType::TRUE)
        {
            m_Index++;
            primaryExpr = std::make_unique<BooleanNode>(true, token.Range);
        }
        else if (token.Type == TokenType::FALSE)
        {
            m_Index++;
            primaryExpr = std::make_unique<BooleanNode>(false, token.Range);
        }
        else if (token.Type == TokenType::INTEGER)
        {
            m_Index++;
            primaryExpr = std::make_unique<IntegerNode>(std::stoll(token.Value), token.Range);
        }
        else if (token.Type == TokenType::FLOAT)
        {
            m_Index++;
            primaryExpr = std::make_unique<FloatNode>(std::stof(token.Value), token.Range);
        }

        else if (token.Type == TokenType::STRING)
        {
            m_Index++;
            primaryExpr = std::make_unique<StringNode>(token.Value, token.Range);
        }
        else if (token.Type == TokenType::LBRACE)
            primaryExpr = ParseListLiteral();
        else if (token.Type == TokenType::LPAREN)
        {
            m_Index++;
            auto expr = Expression();
            if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::RPAREN)
            {
                ReportError("Expected ')' after expression.", token.Range);
                throw ParserSyncException();
            }
            m_Index++;
            primaryExpr = std::move(expr);
        }
        else if (token.Type == TokenType::LCURLY)
            primaryExpr = ParseMapLiteral();
        else if (token.Type == TokenType::MINUS)
        {
            m_Index++;
            NodePtr operand = Factor();
            auto zero = std::make_unique<IntegerNode>(0, token.Range);
            primaryExpr = std::make_unique<BinaryExpressionNode>(TokenType::MINUS, std::move(zero), std::move(operand), token.Range);
        }
        else if (token.Type == TokenType::FUNCTION)
        {
            primaryExpr = ParseFunctionLiteral();
        }
        else if (token.Type == TokenType::IDENTIFIER)
        {
            m_Index++;
            primaryExpr = std::make_unique<IdentifierNode>(token.Value, token.Range);
        }
        else if (token.Type == TokenType::IMPORT)
        {
            m_Index++;
            if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::STRING)
            {
                auto pathStr = m_Tokens[m_Index++].Value;
                return std::make_unique<ImportModuleNode>(pathStr, token.Range);
            }
            ReportError("Expected module name string after 'Import'.", token.Range);
            throw ParserSyncException();
        }
        else
        {
            ReportError("Unexpected token: " + token.Value, token.Range);
            throw ParserSyncException();
        }

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
                    {
                        ReportError("Expected ',' between function arguments.", token.Range);
                        throw ParserSyncException();
                    }
                    if (expectCommaArgs)
                        m_Index++;
                    if (m_Tokens[m_Index].Type != TokenType::RPAREN)
                        args.push_back(Expression());
                    expectCommaArgs = true;
                } while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type == TokenType::COMMA);

                if (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::RPAREN)
                {
                    ReportError("Expected ')' after function arguments.", token.Range);
                    throw ParserSyncException();
                }

                m_Index++;

                primaryExpr = std::make_unique<FunctionCallNode>(std::move(primaryExpr), std::move(args), token.Range);
            }
            else if (m_Tokens[m_Index].Type == TokenType::LBRACE)
            {
                m_Index++;
                auto indexExpr = Expression();
                if (m_Tokens[m_Index].Type != TokenType::RBRACE)
                {
                    ReportError("Expected ']' after list/map index expression.", token.Range);
                    throw ParserSyncException();
                }
                m_Index++;
                primaryExpr = std::make_unique<ListAccessNode>(std::move(primaryExpr), std::move(indexExpr), token.Range);
            }
            else if (m_Tokens[m_Index].Type == TokenType::DOT)
            {
                Token dotToken = m_Tokens[m_Index];;
                m_Index++;

                if (m_Index >= m_Tokens.size() || m_Tokens[m_Index].Type != TokenType::IDENTIFIER)
                {
                    ReportError("Expected member name after '.'", token.Range);
                    throw ParserSyncException();
                }

                Token memberToken = m_Tokens[m_Index];
                m_Index++;

                primaryExpr = std::make_unique<MemberAccessNode>(std::move(primaryExpr), memberToken, dotToken.Range);
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
        return {TokenType::END_OF_FILE, "", m_Tokens[m_Index].Range};
    }

    void Parser::ReportError(const std::string &msg, SourceRange loc)
    {
        m_Errors.emplace_back(msg, loc);
    }

    void Parser::Synchronize()
    {
        m_Index++;

        while (m_Index < m_Tokens.size() && m_Tokens[m_Index].Type != TokenType::END_OF_FILE)
        {
            switch (m_Tokens[m_Index].Type)
            {
                case TokenType::FUNCTION:
                case TokenType::IF:
                case TokenType::FOR:
                case TokenType::WHILE:
                case TokenType::RETURN:
                case TokenType::BREAK:
                case TokenType::CONTINUE:
                case TokenType::END:
                case TokenType::ELSE:
                    return;
                default:
                    m_Index++;
            }
        }
    }
}