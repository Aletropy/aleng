#include "Visitor.h"

#include <sstream>

namespace Aleng
{
    EvaluatedValue Visitor::Visit(const IntegerNode &node)
    {
        return EvaluatedValue(static_cast<double>(node.Value));
    }
    EvaluatedValue Visitor::Visit(const FloatNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const StringNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const BinaryExpressionNode &node)
    {
        auto left = node.Left->Accept(*this);
        auto right = node.Right->Accept(*this);

        EvaluatedValue finalValue;

        std::visit(
            overloads{
                [&](double l, double r)
                {
                    switch (node.Operator)
                    {
                    case TokenType::PLUS:
                        finalValue = l + r;
                        break;
                    case TokenType::MINUS:
                        finalValue = l - r;
                        break;
                    case TokenType::MULTIPLY:
                        finalValue = l * r;
                        break;
                    case TokenType::DIVIDE:
                        if (r == 0.0)
                            throw std::runtime_error("Division by 0 is genious! Congratulations!");
                        finalValue = l / r;
                        break;
                    default:
                        throw std::runtime_error("Unknown operator for binary expression: " + TokenTypeToString(node.Operator));
                        break;
                    }
                },
                [&](std::string l, std::string r)
                {
                    if (node.Operator != TokenType::PLUS)
                        throw std::runtime_error("Only concatenation operator for strings supported.");
                    finalValue = l + r;
                    return;
                },
                [&](std::string l, double r)
                {
                    auto ss = std::stringstream();

                    switch (node.Operator)
                    {
                    case TokenType::PLUS:
                        ss << l;
                        ss << std::to_string(r);
                        finalValue = ss.str();
                        break;
                    case TokenType::MULTIPLY:
                        for (int i = 0; i < static_cast<int>(r); i++)
                            ss << l;
                        finalValue = ss.str();
                        break;
                    default:
                        throw std::runtime_error("Unknown operator for binary expression: " + TokenTypeToString(node.Operator));
                        break;
                    }
                },
                [&](auto &l, auto &r)
                {
                    throw std::runtime_error("Unsupported operand types for operator " + TokenTypeToString(node.Operator) +
                                             ". Left type index: " + std::to_string(left.index()) +
                                             ", Right type index: " + std::to_string(right.index()));
                }},
            left, right);
        return finalValue;
    }
}