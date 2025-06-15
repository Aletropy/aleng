#pragma once

#include "AST.h"

namespace Aleng
{
    class Visitor
    {
    public:
        Visitor() = default;

        EvaluatedValue Visit(const IntegerNode &node);
        EvaluatedValue Visit(const FloatNode &node);
        EvaluatedValue Visit(const BinaryExpressionNode &node);
    };
}