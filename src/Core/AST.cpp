#include "AST.h"
#include "Visitor.h"

namespace Aleng
{
    EvaluatedValue ProgramNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue IfNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue BlockNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue IntegerNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue FloatNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue EqualsExpressionNode::Accept(Visitor &visitor) const
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
    EvaluatedValue IdentifierNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }
    EvaluatedValue AssignExpressionNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }
}