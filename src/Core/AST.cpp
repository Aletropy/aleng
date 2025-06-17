#include "AST.h"
#include "Visitor.h"

namespace Aleng
{
    void PrintEvaluatedValue(EvaluatedValue &value)
    {
        if (auto pvalue = std::get_if<double>(&value))
            std::cout << *pvalue << std::endl;
        if (auto svalue = std::get_if<std::string>(&value))
            std::cout << *svalue << std::endl;
    }

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

    EvaluatedValue FunctionCallNode::Accept(Visitor &Visitor) const
    {
        return Visitor.Visit(*this);
    }

    EvaluatedValue BinaryExpressionNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue ImportModuleNode::Accept(Visitor &visitor) const
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