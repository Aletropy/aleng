#pragma once

#include <string>
#include <iostream>
#include <memory>
#include "Tokens.h"

namespace Aleng
{
    struct ASTNode;
    using NodePtr = std::unique_ptr<ASTNode>;
}

namespace Aleng
{
    std::ostream &operator<<(std::ostream &os, const ASTNode &node);

    using EvaluatedValue = double;
    class Visitor;

    struct ASTNode
    {
        ASTNode() = default;
        virtual ~ASTNode() = default;

        virtual void Print(std::ostream &os) const
        {
            os << "Empty ASTNode";
        }
        virtual EvaluatedValue Accept(Visitor &visitor) const = 0;
    };

    inline std::ostream &operator<<(std::ostream &os, const ASTNode &node)
    {
        node.Print(os);
        return os;
    }

    struct BinaryExpressionNode : ASTNode
    {
        NodePtr Left;
        NodePtr Right;
        TokenType Operator;

        BinaryExpressionNode(TokenType op, NodePtr left, NodePtr right)
            : Operator(op), Left(std::move(left)), Right(std::move(right))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "(";
            os << *Left;
            os << " " << TokenTypeToString(Operator) << " "; // Usa a função helper
            os << *Right;
            os << ")";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct IntegerNode : ASTNode
    {
        int Value;

        IntegerNode()
            : Value(0)
        {
        }

        IntegerNode(int value)
            : Value(value)
        {
        }

        void Print(std::ostream &os) const override
        {
            os << Value;
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct FloatNode : ASTNode
    {
        float Value;

        FloatNode()
            : Value(0.0f)
        {
        }

        FloatNode(float value)
            : Value(value)
        {
        }

        void Print(std::ostream &os) const override
        {
            os << Value << "f";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct StringNode : ASTNode
    {
        std::string Value;

        StringNode()
            : Value("")
        {
        }

        StringNode(const std::string &value)
            : Value(value)
        {
        }

        StringNode(std::string &&value)
            : Value(std::move(value))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "\"" << Value << "\"";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };
} // namespace Aleng