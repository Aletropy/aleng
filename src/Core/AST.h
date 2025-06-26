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
    struct ListRecursiveWrapper;

    using EvaluatedValue = std::variant<
        double,
        std::string,
        bool,
        std::shared_ptr<ListRecursiveWrapper>>;

    void PrintEvaluatedValue(const EvaluatedValue &value);

    struct ListRecursiveWrapper
    {
        std::vector<EvaluatedValue> elements;

        ListRecursiveWrapper() = default;
        ListRecursiveWrapper(std::vector<EvaluatedValue> elems) : elements(std::move(elems)) {}
    };

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
        virtual NodePtr Clone() const = 0;
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

        NodePtr Clone() const override
        {
            auto cloned = std::make_unique<ProgramNode>();
            for (const auto &stmt : Statements)
            {
                if (stmt)
                    cloned->Statements.push_back(stmt->Clone());
            }
            return cloned;
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

        NodePtr Clone() const override
        {
            std::vector<NodePtr> clonedStatements;
            for (const auto &stmt : Statements)
            {
                if (stmt)
                    clonedStatements.push_back(stmt->Clone());
            }
            return std::make_unique<BlockNode>(std::move(clonedStatements));
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
        IfNode(const IfNode &other)
            : Condition(other.Condition ? other.Condition->Clone() : nullptr),
              ThenBranch(other.ThenBranch ? other.ThenBranch->Clone() : nullptr),
              ElseBranch(other.ElseBranch ? other.ElseBranch->Clone() : nullptr) {}

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

        NodePtr Clone() const override
        {
            return std::make_unique<IfNode>(
                Condition ? Condition->Clone() : nullptr,
                ThenBranch ? ThenBranch->Clone() : nullptr,
                ElseBranch ? ElseBranch->Clone() : nullptr);
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
        FunctionDefinitionNode(const FunctionDefinitionNode &other)
            : FunctionName(other.FunctionName),
              Parameters(other.Parameters),
              Body(other.Body ? other.Body->Clone() : nullptr)
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

        NodePtr Clone() const override
        {
            return std::make_unique<FunctionDefinitionNode>(*this);
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
        FunctionCallNode(const FunctionCallNode &other) : Name(other.Name)
        {
            for (const auto &arg : other.Arguments)
            {
                Arguments.push_back(arg ? arg->Clone() : nullptr);
            }
        }

        void Print(std::ostream &os) const override
        {
            os << Name << "(";
            for (auto &arg : Arguments)
                arg->Print(os);
            os << ")\n";
        }

        NodePtr Clone() const override
        {
            std::vector<NodePtr> clonedArgs;
            for (const auto &arg : Arguments)
            {
                if (arg)
                    clonedArgs.push_back(arg->Clone());
            }
            return std::make_unique<FunctionCallNode>(Name, std::move(clonedArgs));
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
        EqualsExpressionNode(const EqualsExpressionNode &other)
            : Left(other.Left ? other.Left->Clone() : nullptr),
              Right(other.Right ? other.Right->Clone() : nullptr),
              Inverse(other.Inverse) {}

        void Print(std::ostream &os) const override
        {
            Left->Print(os);
            os << Inverse ? "!=" : "==";
            Right->Print(os);
            os << "\n";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<EqualsExpressionNode>(
                Left ? Left->Clone() : nullptr,
                Right ? Right->Clone() : nullptr,
                Inverse);
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
        BinaryExpressionNode(const BinaryExpressionNode &other)
            : Operator(other.Operator),
              Left(other.Left ? other.Left->Clone() : nullptr),
              Right(other.Right ? other.Right->Clone() : nullptr) {}

        void Print(std::ostream &os) const override
        {
            os << "(";
            os << *Left;
            os << " " << TokenTypeToString(Operator) << " "; // Usa a função helper
            os << *Right;
            os << ")";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<BinaryExpressionNode>(
                Operator,
                Left ? Left->Clone() : nullptr,
                Right ? Right->Clone() : nullptr);
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
        ImportModuleNode(const ImportModuleNode &other) : ModuleName(other.ModuleName) {}

        void Print(std::ostream &os) const override
        {
            os << "Module " << ModuleName << "\n";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<ImportModuleNode>(ModuleName);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct AssignExpressionNode : ASTNode
    {
        NodePtr Left;
        NodePtr Right;

        AssignExpressionNode(NodePtr left, NodePtr right)
            : Left(std::move(left)), Right(std::move(right))
        {
        }
        AssignExpressionNode(const AssignExpressionNode &other)
            : Left(other.Left ? other.Left->Clone() : nullptr),
              Right(other.Right ? other.Right->Clone() : nullptr) {}

        void Print(std::ostream &os) const override
        {
            os << "(";
            os << *Left;
            os << " = ";
            os << *Right;
            os << ")";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<AssignExpressionNode>(
                Left ? Left->Clone() : nullptr,
                Right ? Right->Clone() : nullptr);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ListAccessNode : ASTNode
    {
        NodePtr Object;
        NodePtr Index;

        ListAccessNode(NodePtr obj, NodePtr index)
            : Object(std::move(obj)), Index(std::move(index)) {}

        void Print(std::ostream &os) const override
        {
            os << *Object;
            os << "[";
            os << *Index;
            os << "]";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<ListAccessNode>(
                Object ? Object->Clone() : nullptr,
                Index ? Index->Clone() : nullptr);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ListNode : ASTNode
    {
        std::vector<NodePtr> Elements;
        ListNode(std::vector<NodePtr> elements)
            : Elements(std::move(elements)) {}

        void Print(std::ostream &os) const override
        {
            os << "[";
            for (size_t i = 0; i < Elements.size(); i++)
            {
                if (Elements[i])
                    Elements[i]->Print(os);
                else
                    os << "<null_expression>";

                if (i < Elements.size() - 1)
                    os << ", ";
            }
            os << "]";
        }

        NodePtr Clone() const override
        {
            std::vector<NodePtr> clonedElements;
            for (const auto &elem : Elements)
            {
                if (elem)
                    clonedElements.push_back(elem->Clone());
            }
            return std::make_unique<ListNode>(std::move(clonedElements));
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct BooleanNode : ASTNode
    {
        bool Value;
        BooleanNode(bool val) : Value(val) {}

        void Print(std::ostream &os) const override { os << (Value ? "true" : "false"); }
        NodePtr Clone() const override { return std::make_unique<BooleanNode>(Value); }

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
        IntegerNode(const IntegerNode &other) : Value(other.Value) {}

        void Print(std::ostream &os) const override
        {
            os << Value;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<IntegerNode>(Value);
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

        FloatNode(const FloatNode &other) : Value(other.Value) {}

        void Print(std::ostream &os) const override
        {
            os << Value << "f";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<FloatNode>(Value);
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

        StringNode(const StringNode &other) : Value(other.Value) {}

        void Print(std::ostream &os) const override
        {
            os << "\"" << Value << "\"";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<StringNode>(Value);
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

        IdentifierNode(const IdentifierNode &other) : Value(other.Value) {}

        void Print(std::ostream &os) const override
        {
            os << Value;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<IdentifierNode>(Value);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };
} // namespace Aleng