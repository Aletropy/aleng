#pragma once

#include "AST.h"
#include <unordered_map>

namespace Aleng
{
    class Visitor
    {
    public:
        Visitor() = default;

        EvaluatedValue Visit(const ProgramNode &node);
        EvaluatedValue Visit(const BlockNode &node);
        EvaluatedValue Visit(const IfNode &node);
        EvaluatedValue Visit(const IntegerNode &node);
        EvaluatedValue Visit(const FloatNode &node);
        EvaluatedValue Visit(const StringNode &node);
        EvaluatedValue Visit(const IdentifierNode &node);
        EvaluatedValue Visit(const AssignExpressionNode &node);
        EvaluatedValue Visit(const BinaryExpressionNode &node);
        EvaluatedValue Visit(const EqualsExpressionNode &node);

    private:
        bool IsTruthy(const EvaluatedValue &val);

    private:
        std::unordered_map<std::string, EvaluatedValue> m_Values;
    };

    template <class... Ts>
    struct overloads : Ts...
    {
        using Ts::operator()...;
    };
}