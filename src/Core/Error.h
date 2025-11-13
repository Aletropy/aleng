#pragma once

#include <stdexcept>
#include <string>
#include "Tokens.h"
#include "AST.h"

namespace Aleng
{
    class AlengError final : public std::runtime_error
    {
    public:
        AlengError(const std::string &message, TokenLocation location)
            : std::runtime_error(message), m_Location(location) {}
        AlengError(const std::string &message, const ASTNode &node)
            : std::runtime_error(message), m_Location(node.Location) {}

        [[nodiscard]] TokenLocation GetLocation() const
        {
            return m_Location;
        }

    private:
        TokenLocation m_Location;
    };

    void PrintFormattedError(const AlengError &err, const std::string &sourceCode, const std::string &filePath);
}