#pragma once

#include "AST.h"
#include <map>
#include <unordered_map>

namespace Aleng
{
    class Visitor
    {
    public:
        Visitor() = default;
        void LoadModuleFiles(std::map<std::string, fs::path> moduleFiles);

        static EvaluatedValue ExecuteAlengFile(const std::string &filepath, Visitor &visitor);

        EvaluatedValue Visit(const ProgramNode &node);
        EvaluatedValue Visit(const BlockNode &node);
        EvaluatedValue Visit(const IfNode &node);
        EvaluatedValue Visit(const IntegerNode &node);
        EvaluatedValue Visit(const FloatNode &node);
        EvaluatedValue Visit(const StringNode &node);
        EvaluatedValue Visit(const IdentifierNode &node);
        EvaluatedValue Visit(const AssignExpressionNode &node);
        EvaluatedValue Visit(const FunctionCallNode &node);
        EvaluatedValue Visit(const ImportModuleNode &node);
        EvaluatedValue Visit(const BinaryExpressionNode &node);
        EvaluatedValue Visit(const EqualsExpressionNode &node);

    private:
        bool IsTruthy(const EvaluatedValue &val);

    private:
        std::unordered_map<std::string, EvaluatedValue> m_Values;
        std::vector<std::string> m_ImportedModules;
        std::map<std::string, fs::path> m_AvailableModules;
    };

    template <class... Ts>
    struct overloads : Ts...
    {
        using Ts::operator()...;
    };
}