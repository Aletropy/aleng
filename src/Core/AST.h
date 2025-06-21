#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <variant>
#include <vector>
#include <optional>
#include "Tokens.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace Aleng
{
    struct ASTNode;
    using NodePtr = std::unique_ptr<ASTNode>;
}

namespace Aleng
{
    std::ostream &operator<<(std::ostream &os, const ASTNode &node);

    class Visitor;

    using EvaluatedValue = std::variant<double, std::string>;

    void PrintEvaluatedValue(const EvaluatedValue &value);

    struct ASTNode
    {
        ASTNode() = default;
        virtual ~ASTNode() = default;

        ASTNode(const ASTNode &) = delete;
        ASTNode(ASTNode &&) = default;
        virtual void Print(std::ostream &os) const
        {
            os << "Empty ASTNode";
        }
        virtual EvaluatedValue Accept(Visitor &visitor) const = 0;
    };

    struct Parameter
    {
        std::string Name;
        std::optional<std::string> TypeName;
        bool IsVariadic = false;

        Parameter(std::string name, std::optional<std::string> typeName = std::nullopt,
                  bool isVariadic = false)
            : Name(std::move(name)), TypeName(std::move(typeName)), IsVariadic(isVariadic)
        {
        }
    };

    inline std::ostream &operator<<(std::ostream &os, const ASTNode &node)
    {
        node.Print(os);
        return os;
    }

    struct ProgramNode : ASTNode
    {
    public:
        std::vector<NodePtr> Statements;

        void Print(std::ostream &os) const override
        {
            for (auto &node : Statements)
                node->Print(os);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct BlockNode : ASTNode
    {
        std::vector<NodePtr> Statements;
        BlockNode(std::vector<NodePtr> stmts)
            : Statements(std::move(stmts))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "{\n";
            for (const auto &stmt : Statements)
            {
                if (stmt)
                    stmt->Print(os);
                os << ";\n";
            }
            os << "}";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct IfNode : ASTNode
    {
        NodePtr Condition;
        NodePtr ThenBranch;
        NodePtr ElseBranch;

        IfNode(NodePtr cond, NodePtr thenB, NodePtr elseB = nullptr)
            : Condition(std::move(cond)), ThenBranch(std::move(thenB)), ElseBranch(std::move(elseB))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "If ";
            Condition->Print(os);
            os << "\n";
            ThenBranch->Print(os);
            if (ElseBranch != nullptr)
            {
                os << "\n";
                ElseBranch->Print(os);
            }
            os << "\n";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct FunctionDefinitionNode : ASTNode
    {
        std::string FunctionName;
        std::vector<Parameter> Parameters;
        NodePtr Body;

        FunctionDefinitionNode(std::string funcName, std::vector<Parameter> params, NodePtr body)
            : FunctionName(funcName), Parameters(std::move(params)), Body(std::move(body))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "Fn " << FunctionName << "(";
            for (size_t i = 0; i < Parameters.size(); ++i)
            {
                if (Parameters[i].IsVariadic)
                    os << "$";
                os << Parameters[i].Name;
                if (Parameters[i].TypeName)
                {
                    os << ": " << *Parameters[i].TypeName;
                }
                if (i < Parameters.size() - 1)
                    os << ", ";
            }
            os << ") { ";
            if (Body)
                Body->Print(os);
            os << " } End";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct FunctionCallNode : ASTNode
    {
        std::string Name;
        std::vector<NodePtr> Arguments;

        FunctionCallNode(const std::string &name, std::vector<NodePtr> args)
            : Name(name), Arguments(std::move(args))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << Name << "(";
            for (auto &arg : Arguments)
                arg->Print(os);
            os << ")\n";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct EqualsExpressionNode : ASTNode
    {
        NodePtr Left;
        NodePtr Right;
        bool Inverse;

        EqualsExpressionNode(NodePtr left, NodePtr right, bool inv = false)
            : Left(std::move(left)), Right(std::move(right)), Inverse(inv)
        {
        }

        void Print(std::ostream &os) const override
        {
            Left->Print(os);
            os << Inverse ? "!=" : "==";
            Right->Print(os);
            os << "\n";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

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

    struct ImportModuleNode : ASTNode
    {
        std::string ModuleName;

        ImportModuleNode(std::string moduleName)
            : ModuleName(std::move(moduleName))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "Module " << ModuleName << "\n";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct AssignExpressionNode : ASTNode
    {
        std::string Left;
        NodePtr Right;

        AssignExpressionNode(std::string left, NodePtr right)
            : Left(std::move(left)), Right(std::move(right))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << "(";
            os << Left;
            os << " = ";
            os << *Right;
            os << ")";
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct IntegerNode : ASTNode
    {
        long long Value;

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

    struct IdentifierNode : ASTNode
    {
        std::string Value;

        IdentifierNode()
            : Value("")
        {
        }

        IdentifierNode(const std::string &value)
            : Value(value)
        {
        }

        IdentifierNode(std::string &&value)
            : Value(std::move(value))
        {
        }

        void Print(std::ostream &os) const override
        {
            os << Value;
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };
} // namespace Aleng