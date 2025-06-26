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
        while (m_Input[m_Index] == ' ' || m_Input[m_Index] == '\t' || m_Input[m_Index] == '\n')
        {
            m_Index++;
        }

        if (m_Index >= m_Input.length())
        {
            return {TokenType::END_OF_FILE, ""};
        }

        if (m_Input[m_Index] == '#')
        {
            if (m_Index + 1 < m_Input.length() && m_Input[m_Index + 1] == '#')
            {
                m_Index += 2;
                while (m_Index < m_Input.size() && m_Input[m_Index] != '#')
                    m_Index++;

                if (m_Index + 1 >= m_Input.length() || m_Input[m_Index + 1] != '#')
                    throw std::runtime_error("Expected double '##' to end multiple line comment.");
                m_Index += 2;
            }

            m_Index++;
            while (m_Index < m_Input.size() && m_Input[m_Index] != '\n')
                m_Index++;
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
                if (m_Index + 1 < m_Input.length() && m_Input[m_Index + 1] == '.')
                    return {TokenType::INTEGER, value};

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

            while (std::isalnum(m_Input[m_Index]) || m_Input[m_Index] == '_')
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
            else if (value == "True")
                return {TokenType::TRUE, value};
            else if (value == "False")
                return {TokenType::FALSE, value};
            else if (value == "in")
                return {TokenType::IN, value};
            else if (value == "until")
                return {TokenType::UNTIL, value};
            else if (value == "step")
                return {TokenType::STEP, value};

            return {TokenType::IDENTIFIER, value};
        }

        if (c == '$')
        {
            m_Index++;
            return {TokenType::DOLLAR, '$'};
        }

        if (c == '.')
        {
            if (m_Index + 1 < m_Input.length() && m_Input[m_Index] == '.')
            {
                m_Index += 2;
                return {TokenType::RANGE, ".."};
            }
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