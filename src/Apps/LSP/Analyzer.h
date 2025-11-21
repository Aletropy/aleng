#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Core/AST.h"


namespace AlengLSP
{
    struct SymbolInfo
    {
        std::string Name;
        std::string Kind;
        int lineDefined;
    };

    struct ScopeRange
    {
        int StartLine;
        int EndLine;
        std::vector<SymbolInfo> Symbols;
        std::shared_ptr<ScopeRange> Parent;
        std::vector<std::shared_ptr<ScopeRange>> Children;

        [[nodiscard]] bool Contains(const int line) const
        {
            return line >= StartLine && (EndLine == -1 || line <= EndLine);
        }
    };

    class Analyzer
    {
    public:
        void Analyze(const Aleng::ProgramNode& program);

        std::vector<SymbolInfo> GetSymbolsAt(int line) const;

    private:
        void VisitNode(const Aleng::ASTNode* node);
        void VisitBlock(const Aleng::BlockNode* node);

        std::shared_ptr<ScopeRange> m_RootScope;
        std::shared_ptr<ScopeRange> m_CurrentScope;
    };
} // AlengLSP