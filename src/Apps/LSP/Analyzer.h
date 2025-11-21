#pragma once

#include "Core/AST.h"
#include "Core/Visitor.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm> // Para std::sort
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AlengLSP
{
    struct TypeInfo {
        std::string Name;
        std::vector<std::string> MapKeys;
        std::string Signature;
    };

    struct SymbolInfo {
        std::string Name;
        std::string Kind;
        int LineDefined;
        TypeInfo Type;
    };

    struct ScopeRange {
        int StartLine{};
        int EndLine{};
        std::vector<SymbolInfo> Symbols;
        std::shared_ptr<ScopeRange> Parent;
        std::vector<std::shared_ptr<ScopeRange>> Children;
    };

    class SemanticTokenBuilder {
        struct RawToken {
            int line;
            int startChar;
            int length;
            int type;
            int modifiers;

            bool operator<(const RawToken& other) const {
                if (line != other.line) return line < other.line;
                return startChar < other.startChar;
            }
        };

        std::vector<RawToken> m_RawTokens;

    public:
        void Push(const int line, const int startChar, const int length, const int type, const int modifiers = 0) {
            if (length <= 0 || line < 0 || startChar < 0) return;
            m_RawTokens.push_back({line, startChar, length, type, modifiers});
        }

        json GetData() {
            std::sort(m_RawTokens.begin(), m_RawTokens.end());

            std::vector<int> data;
            int prevLine = 0;
            int prevChar = 0;

            for (const auto&[line, startChar, length, type, modifiers] : m_RawTokens) {
                int deltaLine = line - prevLine;
                int deltaChar = (deltaLine == 0) ? (startChar - prevChar) : startChar;

                data.push_back(deltaLine);
                data.push_back(deltaChar);
                data.push_back(length);
                data.push_back(type);
                data.push_back(modifiers);

                prevLine = line;
                prevChar = startChar;
            }

            return data;
        }
    };


    class Analyzer {
    public:
        Analyzer() {
            m_RootScope = std::make_shared<ScopeRange>();
            m_RootScope->StartLine = 0;
            m_RootScope->EndLine = -1;
            m_CurrentScope = m_RootScope;
        }

        void Analyze(const Aleng::ProgramNode& program);

        [[nodiscard]] std::vector<SymbolInfo> GetSymbolsAt(int line) const;
        [[nodiscard]] const SymbolInfo* FindSymbol(const std::string& name, int line) const;

        json GetSemanticTokens();

    private:
        void VisitNode(const Aleng::ASTNode* node);
        void VisitBlock(const Aleng::BlockNode* node);

        void UpdateOrDefineSymbol(const std::string& name, const std::string& kind, int line, const TypeInfo& type) const;

        std::shared_ptr<ScopeRange> m_RootScope;
        std::shared_ptr<ScopeRange> m_CurrentScope;

        SemanticTokenBuilder m_TokenBuilder;
    };

} // namespace AlengLSP