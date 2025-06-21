#include "Lexer.h"

#include <iostream>

namespace Aleng
{
    Lexer::Lexer(const std::string &input)
        : m_Input(input), m_Index(0)
    {
    }

    Token Lexer::Next()
    {
        if (m_Index >= m_Input.length())
        {
            return {TokenType::END_OF_FILE, ""};
        }

        while (m_Input[m_Index] == ' ' || m_Input[m_Index] == '\t' || m_Input[m_Index] == '\n')
        {
            m_Index++;
        }

        auto c = m_Input[m_Index];

        // Number verification
        if (std::isdigit(c))
        {
            std::string value = "";

            while (m_Index < m_Input.length() && std::isdigit(m_Input[m_Index]))
            {
                value += m_Input[m_Index];
                m_Index++;
            }

            if (m_Input[m_Index] == '.')
            {
                m_Index++;
                value += ".";
                while (m_Index < m_Input.length() && std::isdigit(m_Input[m_Index]))
                {
                    value += m_Input[m_Index];
                    m_Index++;
                }

                return {TokenType::FLOAT, value};
            }

            return {TokenType::INTEGER, value};
        }

        if (std::isalpha(c) || c == '_')
        {
            std::string value = "";

            while (std::isalpha(m_Input[m_Index]))
            {
                value += m_Input[m_Index];
                m_Index++;
            }

            if (value == "If")
                return {TokenType::IF, value};
            else if (value == "Else")
                return {TokenType::ELSE, value};
            else if (value == "While")
                return {TokenType::WHILE, value};
            else if (value == "For")
                return {TokenType::FOR, value};
            else if (value == "Fn")
                return {TokenType::FUNCTION, value};
            else if (value == "Module")
                return {TokenType::MODULE, value};
            else if (value == "End")
                return {TokenType::END, value};

            return {TokenType::IDENTIFIER, value};
        }

        if (c == '$')
        {
            m_Index++;
            return {TokenType::DOLLAR, '$'};
        }

        if (c == '"')
        {
            m_Index++; // Consume "
            std::string value = "";

            while (m_Index < m_Input.length() && m_Input[m_Index] != '"')
            {
                if (m_Input[m_Index] == '\\')
                {
                    m_Index++;
                    if (m_Index < m_Input.length())
                    {
                        switch (m_Input[m_Index])
                        {
                        case 'n':
                            value += '\n';
                            break;
                        case 't':
                            value += '\t';
                            break;
                        case '"':
                            value += '"';
                            break;
                        case '\\':
                            value += '\\';
                            break;
                        default:
                            value += m_Input[m_Index]; // Or throw error
                        }
                    }
                }
                else
                    value += m_Input[m_Index];

                m_Index++;
            }

            if (m_Input[m_Index] != '"')
            {
                throw std::runtime_error("Expected string termination.");
                return {TokenType::UNKNOWN, c};
            }

            m_Index++;
            return {TokenType::STRING, value};
        }

        if (c == '=')
        {
            if (m_Input[m_Index + 1] == '=')
            {
                m_Index += 2;
                return {TokenType::EQUALS, "=="};
            }

            m_Index++;
            return {TokenType::ASSIGN, c};
        }

        if (c == '!')
        {
            if (m_Input[m_Index + 1] == '=')
            {
                m_Index += 2;
                return {TokenType::EQUALS, "!="};
            }

            throw std::runtime_error("NOT not supported yet.");
            return {TokenType::UNKNOWN, ""};
        }

        // General characters
        switch (c)
        {
        case '+':
            m_Index++;
            return {TokenType::PLUS, c};
        case '-':
            m_Index++;
            return {TokenType::MINUS, c};
        case '*':
            m_Index++;
            return {TokenType::MULTIPLY, c};
        case '/':
            m_Index++;
            return {TokenType::DIVIDE, c};
        case '^':
            m_Index++;
            return {TokenType::POWER, c};
        case ',':
            m_Index++;
            return {TokenType::COMMA, c};
        case ';':
            m_Index++;
            return {TokenType::SEMICOLON, c};
        case ':':
            m_Index++;
            return {TokenType::COLON, c};
        case '(':
            m_Index++;
            return {TokenType::LPAREN, c};
        case ')':
            m_Index++;
            return {TokenType::RPAREN, c};
        case '{':
            m_Index++;
            return {TokenType::LCURLY, c};
        case '}':
            m_Index++;
            return {TokenType::RCURLY, c};
        case '[':
            m_Index++;
            return {TokenType::LBRACE, c};
        case ']':
            m_Index++;
            return {TokenType::RBRACE, c};
        case '>':
            m_Index++;
            return {TokenType::GREATER, c};
        case '<':
            m_Index++;
            return {TokenType::MINOR, c};
        }

        m_Index++;
        return {TokenType::UNKNOWN, c};
    }

    std::vector<Token> Lexer::Tokenize()
    {
        auto tokens = std::vector<Token>();
        while (m_Index < m_Input.length())
        {
            tokens.push_back(Next());
        }
        tokens.push_back({TokenType::END_OF_FILE, ""});
        return tokens;
    }
}