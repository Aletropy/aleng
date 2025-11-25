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
        AlengError(const std::string &message, SourceRange location)
            : std::runtime_error(message), m_Range(std::move(location)) {}
        AlengError(const std::string &message, const ASTNode &node)
            : std::runtime_error(message), m_Range(node.Location) {}

        [[nodiscard]] SourceRange GetRange() const
        {
            return m_Range;
        }

    private:
        SourceRange m_Range;
    };

    void PrintFormattedError(const AlengError &err, const std::string &sourceCode);

}