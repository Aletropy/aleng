#pragma once

#include "Core/AST.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AlengLSP
{
    enum class SemanticType
    {
        None = -1,
        Variable = 0,
        Function = 1,
        Parameter = 2,
        Property = 3,
        Class = 4,
        String = 5,
        Number = 6,
        Keyword = 7,
        Operator = 8
    };

    struct TypeInfo
    {
        enum class Kind { Unknown, Any, Void, Number, String, Boolean, List, Map, Function };

        Kind kind = Kind::Unknown;
        std::shared_ptr<TypeInfo> innerType; // For List<T>
        std::unordered_map<std::string, std::shared_ptr<TypeInfo> > mapStructure; // For structured Maps

        std::vector<std::shared_ptr<TypeInfo> > paramTypes;
        std::shared_ptr<TypeInfo> returnType;

        std::string ToString() const;

        bool operator==(const TypeInfo &other) const;
    };

    struct Symbol
    {
        enum class Category { Variable, Function, Parameter, Property, Class };

        std::string name;
        Category category;
        std::shared_ptr<TypeInfo> type;

        Aleng::SourceRange definitionRange;
        std::vector<Aleng::SourceRange> references; // Tracks all usages for "Find References"

        std::string documentation;

        void AddReference(const Aleng::SourceRange &range);
    };

    class Scope
    {
    public:
        std::shared_ptr<Scope> parent;
        std::unordered_map<std::string, std::shared_ptr<Symbol> > symbols;
        int level = 0;

        explicit Scope(std::shared_ptr<Scope> p = nullptr);

        void Define(const std::shared_ptr<Symbol> &sym);

        std::shared_ptr<Symbol> Resolve(const std::string &name);
    };

    // Represents a semantic unit in the code for fast spatial lookup (Hover/GoTo)
    struct SpatialEntry
    {
        Aleng::SourceRange range;
        std::shared_ptr<Symbol> symbol; // The symbol this node refers to (or defines)
        std::shared_ptr<TypeInfo> type; // The resolved type of this specific expression
        SemanticType customType = SemanticType::None;

        // Comparison for binary search
        bool operator<(const SpatialEntry &other) const
        {
            if (range.Start.Line != other.range.Start.Line)
                return range.Start.Line < other.range.Start.Line;
            return range.Start.Column < other.range.Start.Column;
        }
    };

    struct ScopeEntry
    {
        Aleng::SourceRange range;
        std::shared_ptr<Scope> scope;
    };

    struct FileAnalysisContext
    {
        std::shared_ptr<Scope> globalScope;
        std::vector<SpatialEntry> spatialIndex;
        std::vector<std::shared_ptr<Symbol> > allSymbols; // Keep alive
        std::vector<ScopeEntry> scopeIndex;
    };

    class Analyzer
    {
    public:
        Analyzer() = default;

        void Analyze(const Aleng::ProgramNode &program, const std::string &uri);

        [[nodiscard]] std::shared_ptr<Symbol> FindSymbolAt(const std::string &uri, int line, int col) const;

        [[nodiscard]] std::string GetHoverInfo(const std::string &uri, int line, int col) const;

        [[nodiscard]] std::vector<Aleng::SourceRange> GetReferences(const std::string &uri, int line, int col) const;

        [[nodiscard]] json GetSemanticTokens(const std::string &uri) const;

        [[nodiscard]] std::vector<std::shared_ptr<Symbol> > GetCompletions(
            const std::string &uri, int line, int col) const;

    private:
        std::unordered_map<std::string, FileAnalysisContext> m_Contexts;

        static void RegisterScope(const Aleng::SourceRange& range, std::shared_ptr<Scope> scope, FileAnalysisContext& ctx);

        void VisitNode(const Aleng::ASTNode *node, std::shared_ptr<Scope> &currentScope, FileAnalysisContext &ctx);

        static std::shared_ptr<TypeInfo> InferType(const Aleng::ASTNode *node, const std::shared_ptr<Scope> &scope);

        static void AddToSpatialIndex(const Aleng::ASTNode *node, const std::shared_ptr<Symbol> &sym,
                                      const std::shared_ptr<TypeInfo> &type, FileAnalysisContext &ctx);

        static void AddSpatialToken(const Aleng::SourceRange& range, SemanticType type, FileAnalysisContext& ctx);

        std::shared_ptr<Scope> FindScopeAt(const std::string& uri, int line, int col) const;

        static std::shared_ptr<Symbol> DefineSymbol(const std::string &name, Symbol::Category cat,
                                                    std::shared_ptr<TypeInfo> type, const Aleng::SourceRange &range,
                                                    std::shared_ptr<Scope> &scope, FileAnalysisContext &ctx);
    };
} // namespace AlengLSP
