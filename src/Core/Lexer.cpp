#include "Lexer.h"

#include <iostream>
#include <utility>
#include "Error.h"

namespace Aleng
{
    Lexer::Lexer(std::string input, std::string  filepath)
        : m_Input(std::move(input)), m_FilePath(std::move(filepath))
    {
    }

    Token Lexer::Next()
    {
        TokenLocation startLoc = {m_Line, m_Column, m_FilePath};

        auto advance = [&](const int count = 1)
        {
            for (int i = 0; i < count; ++i)
            {
                if (m_Index < m_Input.length() && m_Input[m_Index] == '\n')
                {
                    m_Line++;
                    m_Column = 1;
                }
                else
                {
                    m_Column++;
                }
                m_Index++;
            }
        };

        while (m_Index < m_Input.length())
        {
            const char currentChar = m_Input[m_Index];

            if (isspace(currentChar))
            {
                advance();
                continue;
            }

            if (currentChar == '#')
            {
                if (m_Index + 1 < m_Input.length() && m_Input[m_Index + 1] == '#')
                {
                    advance(2);
                    startLoc = {m_Line, m_Column, m_FilePath};

                    while (m_Index + 1 < m_Input.length() && !(m_Input[m_Index] == '#' && m_Input[m_Index + 1] == '#'))
                    {
                        advance();
                    }

                    if (m_Index + 1 >= m_Input.length())
                    {
                        throw AlengError("Multiple lines comment was not closed. Expected '##'.", startLoc);
                    }

                    advance(2);
                    continue;
                }

                while (m_Index < m_Input.length() && m_Input[m_Index] != '\n')
                {
                    advance();
                }
                continue;
            }

            break;
        }

        if (m_Index >= m_Input.length())
        {
            return {TokenType::END_OF_FILE, "", startLoc};
        }

        startLoc = {m_Line, m_Column, m_FilePath};

        auto c = m_Input[m_Index];

        // Number verification
        if (std::isdigit(c))
        {
            std::string value;

            while (m_Index < m_Input.length() && std::isdigit(m_Input[m_Index]))
            {
                value += m_Input[m_Index];
                advance();
            }

            startLoc = {m_Line, m_Column, m_FilePath};

            if (m_Index < m_Input.length() && m_Input[m_Index] == '.' &&
             m_Index + 1 < m_Input.length() && std::isdigit(m_Input[m_Index + 1]))
            {
                value += ".";
                advance();
                while (m_Index < m_Input.length() && std::isdigit(m_Input[m_Index]))
                {
                    value += m_Input[m_Index];
                    advance();
                }

                return {
                    TokenType::FLOAT, value, startLoc};
            }

            return {TokenType::INTEGER, value, startLoc};
        }

        if (std::isalpha(c) || c == '_')
        {
            std::string value;

            while (std::isalnum(m_Input[m_Index]) || m_Input[m_Index] == '_')
            {
                value += m_Input[m_Index];
                advance();
            }

            if (value == "If")
                return {TokenType::IF, value, startLoc};
            else if (value == "Else")
                return {TokenType::ELSE, value, startLoc};
            else if (value == "While")
                return {TokenType::WHILE, value, startLoc};
            else if (value == "For")
                return {TokenType::FOR, value, startLoc};
            else if (value == "Fn")
                return {TokenType::FUNCTION, value, startLoc};
            else if (value == "Return")
                return {TokenType::RETURN, value, startLoc};
            else if (value == "Break")
                return {TokenType::BREAK, value, startLoc};
            else if (value == "Continue")
                return {TokenType::CONTINUE, value, startLoc};
            else if (value == "Import")
                return {TokenType::IMPORT, value, startLoc};
            else if (value == "End")
                return {TokenType::END, value, startLoc};
            else if (value == "True")
                return {TokenType::TRUE, value, startLoc};
            else if (value == "False")
                return {TokenType::FALSE, value, startLoc};
            else if (value == "in")
                return {TokenType::IN, value, startLoc};
            else if (value == "until")
                return {TokenType::UNTIL, value, startLoc};
            else if (value == "step")
                return {TokenType::STEP, value, startLoc};
            else if (value == "and")
                return {TokenType::AND, value, startLoc};
            else if (value == "or")
                return {TokenType::OR, value, startLoc};
            else if (value == "not")
                return {TokenType::NOT, value, startLoc};

            return {TokenType::IDENTIFIER, value, startLoc};
        }

        if (c == '$')
        {
            advance();
            return {TokenType::DOLLAR, '$', startLoc};
        }

        if (c == '"')
        {
            advance();
            std::string value;

            while (m_Index < m_Input.length() && m_Input[m_Index] != '"')
            {
                if (m_Input[m_Index] == '\\')
                {
                    advance();
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

                advance();
            }

            if (m_Input[m_Index] != '"')
                throw AlengError("Expected string termination.", startLoc);

            advance();
            return {TokenType::STRING, value, startLoc};
        }

        if (c == '.')
        {
            if (m_Index + 1 < m_Input.length() && m_Input[m_Index + 1] == '.')
            {
                advance(2);
                return {TokenType::RANGE, "..", startLoc};
            }

            advance();
            return {TokenType::DOT, ".", startLoc};
        }

        if (c == '>')
        {
            advance();
            if (m_Index < m_Input.length() && m_Input[m_Index] == '=')
            {
                advance();
                return {TokenType::GREATER_EQUAL, ">=", startLoc};
            }
            return {TokenType::GREATER, ">", startLoc};
        }

        if (c == '<')
        {
            advance();
            if (m_Index < m_Input.length() && m_Input[m_Index] == '=')
            {
                advance();
                return {TokenType::MINOR_EQUAL, "<=", startLoc};
            }
            return {TokenType::MINOR, "<", startLoc};
        }

        if (c == '=')
        {
            if (m_Input[m_Index + 1] == '=')
            {
                advance(2);
                return {TokenType::EQUALS, "==", startLoc};
            }

            advance();
            return {TokenType::ASSIGN, c, startLoc};
        }

        if (c == '!')
        {
            if (m_Input[m_Index + 1] == '=')
            {
                advance(2);
                return {TokenType::EQUALS, "!=", startLoc};
            }
        }

        // General characters
        switch (c)
        {
        case '+':
            advance();
            return {TokenType::PLUS, c, startLoc};
        case '-':
            advance();
            return {TokenType::MINUS, c, startLoc};
        case '*':
            advance();
            return {TokenType::MULTIPLY, c, startLoc};
        case '/':
            advance();
            return {TokenType::DIVIDE, c, startLoc};
        case '^':
            advance();
            return {TokenType::POWER, c, startLoc};
        case '%':
            advance();
            return {TokenType::MODULO, c, startLoc};
        case ',':
            advance();
            return {TokenType::COMMA, c, startLoc};
        case ';':
            advance();
            return {TokenType::SEMICOLON, c, startLoc};
        case ':':
            advance();
            return {TokenType::COLON, c, startLoc};
        case '(':
            advance();
            return {TokenType::LPAREN, c, startLoc};
        case ')':
            advance();
            return {TokenType::RPAREN, c, startLoc};
        case '{':
            advance();
            return {TokenType::LCURLY, c, startLoc};
        case '}':
            advance();
            return {TokenType::RCURLY, c, startLoc};
        case '[':
            advance();
            return {TokenType::LBRACE, c, startLoc};
        case ']':
            advance();
            return {TokenType::RBRACE, c, startLoc};
        default:
            advance();
            return {TokenType::UNKNOWN, c, startLoc};
        }
    }

    std::vector<Token> Lexer::Tokenize()
    {
        auto tokens = std::vector<Token>();
        Token token = Next();
        while (token.Type != TokenType::END_OF_FILE)
        {
            tokens.push_back(token);
            token = Next();
        }
        tokens.push_back(token);
        return tokens;
    }
}