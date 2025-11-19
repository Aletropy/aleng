#include "AST.h"

#include <iostream>

#include "Visitor.h"

namespace Aleng
{
    void PrintEvaluatedValue(const EvaluatedValue &value, bool raw)
    {
        if (auto dvalue = std::get_if<double>(&value)) {
            if (double val = *dvalue; val == static_cast<long long>(val)) {
                std::cout << static_cast<long long>(val);
            } else {
                std::cout << val;
            }
        }
        if (auto svalue = std::get_if<std::string>(&value))
            std::cout << *svalue;
        if (auto bvalue = std::get_if<bool>(&value))
            std::cout << ((*bvalue) ? "True" : "False");
        if (auto fovalue = std::get_if<FunctionStorage>(&value))
            std::cout << "<Function: " << (*fovalue)->Name << ">" << std::endl;
        if (auto lvalue = std::get_if<ListStorage>(&value))
        {
            std::cout << "[";
            for (size_t i = 0; i < (*lvalue)->elements.size(); i++)
            {
                auto &val = (*lvalue)->elements[i];
                if (auto pvalue = std::get_if<double>(&val))
                    std::cout << *pvalue;
                if (auto svalue = std::get_if<std::string>(&val))
                    std::cout << *svalue;
                if (auto bvalue = std::get_if<bool>(&val))
                    std::cout << ((*bvalue) ? "True" : "False");
                if (auto llvalue = std::get_if<ListStorage>(&val))
                    PrintEvaluatedValue(*llvalue, true);
                if (i < (*lvalue)->elements.size() - 1)
                    std::cout << ", ";
            }
            std::cout << "]";
        }
        if (auto mvalue = std::get_if<MapStorage>(&value))
        {
            std::cout << "{";
            auto &map = (*mvalue)->elements;
            auto it = map.begin();
            while (it != map.end())
            {
                std::cout << "\"" << it->first << "\" = ";
                PrintEvaluatedValue(it->second, true);
                ++it;
                if (it != map.end())
                    std::cout << ", ";
            }
            std::cout << "}";
        }

        if (!raw)
            std::cout << std::endl;
    }

    EvaluatedValue ProgramNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue IfNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue ForStatementNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue WhileStatementNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue BlockNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue FunctionDefinitionNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue MapNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue ListNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue BooleanNode::Accept(Visitor &visitor) const
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

    EvaluatedValue ReturnNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue BreakNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue ContinueNode::Accept(Visitor &visitor) const
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

    EvaluatedValue UnaryExpressionNode::Accept(Visitor &visitor) const
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

    EvaluatedValue MemberAccessNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }

    EvaluatedValue ListAccessNode::Accept(Visitor &visitor) const
    {
        return visitor.Visit(*this);
    }
}