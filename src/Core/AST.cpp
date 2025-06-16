#include "AST.h"
#include "Visitor.h"

namespace Aleng
{
    EvaluatedValue IntegerNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue FloatNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue BinaryExpressionNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue StringNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }
}