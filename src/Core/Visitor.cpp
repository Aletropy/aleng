#include "Visitor.h"

namespace Aleng
{
    EvaluatedValue Visitor::Visit(const IntegerNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const FloatNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const BinaryExpressionNode &node)
    {
        auto left = node.Left->Accept(*this);
        auto right = node.Right->Accept(*this);

        switch (node.Operator)
        {
        case TokenType::PLUS:
            return left + right;
        case TokenType::MINUS:
            return left - right;
        case TokenType::MULTIPLY:
            return left * right;
        case TokenType::DIVIDE:
            return left / right;
        default:
            throw std::runtime_error("Unknown operator for binary expression: " + TokenTypeToString(node.Operator));
            break;
        }
    }
}