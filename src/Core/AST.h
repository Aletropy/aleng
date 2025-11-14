#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <variant>
#include <vector>
#include <optional>
#include <unordered_map>
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
    struct MapRecursiveWrapper;
    struct FunctionObject;

    using ListStorage = std::shared_ptr<ListRecursiveWrapper>;
    using MapStorage = std::shared_ptr<MapRecursiveWrapper>;
    using FunctionStorage = std::shared_ptr<FunctionObject>;

    using EvaluatedValue = std::variant<
        double,
        std::string,
        bool,
        ListStorage,
        MapStorage,
        FunctionStorage>;

    using SymbolTableStack = std::vector<std::unordered_map<std::string, EvaluatedValue>>;

    void PrintEvaluatedValue(const EvaluatedValue &value, bool raw = false);

    struct ListRecursiveWrapper
    {
        std::vector<EvaluatedValue> elements;

        ListRecursiveWrapper() = default;
        ListRecursiveWrapper(std::vector<EvaluatedValue> elems) : elements(std::move(elems)) {}
    };

    struct MapRecursiveWrapper
    {
        std::unordered_map<std::string, EvaluatedValue> elements;

        MapRecursiveWrapper() = default;
        MapRecursiveWrapper(std::unordered_map<std::string, EvaluatedValue> elems)
            : elements(std::move(elems))
        {
        }
    };

    struct ASTNode
    {
        TokenLocation Location;

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
        BlockNode(std::vector<NodePtr> stmts, TokenLocation loc)
            : Statements(std::move(stmts))
        {
            this->Location = loc;
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
            return std::make_unique<BlockNode>(std::move(clonedStatements), Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct IfNode : ASTNode
    {
        NodePtr Condition;
        NodePtr ThenBranch;
        NodePtr ElseBranch;

        IfNode(NodePtr cond, NodePtr thenB, NodePtr elseB, TokenLocation loc)
            : Condition(std::move(cond)), ThenBranch(std::move(thenB)), ElseBranch(std::move(elseB))
        {
            this->Location = loc;
        }
        IfNode(const IfNode &other)
            : Condition(other.Condition ? other.Condition->Clone() : nullptr),
              ThenBranch(other.ThenBranch ? other.ThenBranch->Clone() : nullptr),
              ElseBranch(other.ElseBranch ? other.ElseBranch->Clone() : nullptr)
        {
            this->Location = other.Location;
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

        NodePtr Clone() const override
        {
            return std::make_unique<IfNode>(
                Condition ? Condition->Clone() : nullptr,
                ThenBranch ? ThenBranch->Clone() : nullptr,
                ElseBranch ? ElseBranch->Clone() : nullptr, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ForNumericRange
    {
        std::string IteratorVariableName;
        NodePtr StartExpression;
        NodePtr EndExpression;
        NodePtr StepExpression;
        bool IsUntil;

        ForNumericRange(std::string name, NodePtr start, NodePtr end, NodePtr step, bool until)
            : IteratorVariableName(std::move(name)), StartExpression(std::move(start)),
              EndExpression(std::move(end)), StepExpression(std::move(step)), IsUntil(until) {}

        ForNumericRange(const ForNumericRange &other)
            : IteratorVariableName(other.IteratorVariableName),
              StartExpression(other.StartExpression ? other.StartExpression->Clone() : nullptr),
              EndExpression(other.EndExpression ? other.EndExpression->Clone() : nullptr),
              StepExpression(other.StepExpression ? other.StepExpression->Clone() : nullptr),
              IsUntil(other.IsUntil) {}
    };

    struct ForCollectionRange
    {
        std::string IteratorVariableName;
        NodePtr CollectionExpression;

        ForCollectionRange(std::string name, NodePtr collection)
            : IteratorVariableName(std::move(name)), CollectionExpression(std::move(collection)) {}

        ForCollectionRange(const ForCollectionRange &other)
            : IteratorVariableName(other.IteratorVariableName),
              CollectionExpression(other.CollectionExpression ? other.CollectionExpression->Clone() : nullptr) {}
    };

    struct ForStatementNode : ASTNode
    {
        enum class LoopType
        {
            NUMERIC,
            COLLECTION
        };
        LoopType Type;

        std::optional<ForNumericRange> NumericLoopInfo;
        std::optional<ForCollectionRange> CollectionLoopInfo;

        NodePtr Body;

        ForStatementNode(ForNumericRange numericInfo, NodePtr body, TokenLocation loc)
            : Type(LoopType::NUMERIC), NumericLoopInfo(std::move(numericInfo)), Body(std::move(body))
        {
            this->Location = loc;
        }

        ForStatementNode(ForCollectionRange collectionInfo, NodePtr body, TokenLocation loc)
            : Type(LoopType::COLLECTION), CollectionLoopInfo(std::move(collectionInfo)), Body(std::move(body))
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << "For ";
            if (Type == LoopType::NUMERIC && NumericLoopInfo)
            {
                os << NumericLoopInfo->IteratorVariableName << " = ";
                NumericLoopInfo->StartExpression->Print(os);
                os << (NumericLoopInfo->IsUntil ? " until " : " .. ");
                NumericLoopInfo->EndExpression->Print(os);
                if (NumericLoopInfo->StepExpression)
                {
                    os << " step ";
                    NumericLoopInfo->StepExpression->Print(os);
                }
            }
            else if (Type == LoopType::COLLECTION && CollectionLoopInfo)
            {
                os << CollectionLoopInfo->IteratorVariableName << " in ";
                CollectionLoopInfo->CollectionExpression->Print(os);
            }
            os << " {\n";
            if (Body)
                Body->Print(os);
            os << "\n} End";
        }

        NodePtr Clone() const override
        {
            if (Type == LoopType::NUMERIC && NumericLoopInfo)
            {
                ForNumericRange clonedNumericInfo = {
                    NumericLoopInfo->IteratorVariableName,
                    NumericLoopInfo->StartExpression ? NumericLoopInfo->StartExpression->Clone() : nullptr,
                    NumericLoopInfo->EndExpression ? NumericLoopInfo->EndExpression->Clone() : nullptr,
                    NumericLoopInfo->StepExpression ? NumericLoopInfo->StepExpression->Clone() : nullptr,
                    NumericLoopInfo->IsUntil};
                return std::make_unique<ForStatementNode>(
                    std::move(clonedNumericInfo), Body ? Body->Clone() : nullptr, Location);
            }
            else if (Type == LoopType::COLLECTION && CollectionLoopInfo)
            {
                ForCollectionRange clonedCollectionInfo = {
                    CollectionLoopInfo->IteratorVariableName,
                    CollectionLoopInfo->CollectionExpression ? CollectionLoopInfo->CollectionExpression->Clone() : nullptr};
                return std::make_unique<ForStatementNode>(
                    std::move(clonedCollectionInfo), Body ? Body->Clone() : nullptr, Location);
            }
            throw std::runtime_error("Invalid ForStatementNode state for cloning.");
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct WhileStatementNode : ASTNode
    {
        NodePtr Condition;
        NodePtr Body;

        WhileStatementNode(NodePtr cond, NodePtr body, TokenLocation loc)
            : Condition(std::move(cond)), Body(std::move(body))
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << "While ";
            Condition->Print(os);
            os << " {\n";
            if (Body)
                Body->Print(os);
            os << "\n} End";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<WhileStatementNode>(
                Condition ? Condition->Clone() : nullptr,
                Body ? Body->Clone() : nullptr, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct FunctionDefinitionNode : ASTNode
    {
        std::string FunctionName;
        std::vector<Parameter> Parameters;
        NodePtr Body;

        FunctionDefinitionNode(std::string funcName, std::vector<Parameter> params, NodePtr body, TokenLocation loc)
            : FunctionName(funcName), Parameters(std::move(params)), Body(std::move(body))
        {
            this->Location = loc;
        }
        FunctionDefinitionNode(const FunctionDefinitionNode &other)
            : FunctionName(other.FunctionName),
              Parameters(other.Parameters),
              Body(other.Body ? other.Body->Clone() : nullptr)
        {
            this->Location = other.Location;
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
        NodePtr CallableExpression;
        std::vector<NodePtr> Arguments;

        FunctionCallNode(NodePtr calExpr, std::vector<NodePtr> args, TokenLocation loc)
            : CallableExpression(std::move(calExpr)), Arguments(std::move(args))
        {
            this->Location = loc;
        }
        FunctionCallNode(const FunctionCallNode &other) : CallableExpression(other.CallableExpression->Clone())
        {
            this->Location = other.Location;
            for (const auto &arg : other.Arguments)
            {
                Arguments.push_back(arg ? arg->Clone() : nullptr);
            }
        }

        void Print(std::ostream &os) const override
        {
            os << CallableExpression << "(";
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
            return std::make_unique<FunctionCallNode>(CallableExpression->Clone(), std::move(clonedArgs), Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ReturnNode : ASTNode
    {
        NodePtr ReturnValueExpression;

        ReturnNode(NodePtr valExpr, TokenLocation loc) : ReturnValueExpression(std::move(valExpr))
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << "Return";
            os << ReturnValueExpression << std::endl;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<ReturnNode>(
                ReturnValueExpression->Clone(), Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct BreakNode : ASTNode
    {
        BreakNode(TokenLocation loc)
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << "Break" << std::endl;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<BreakNode>(Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ContinueNode : ASTNode
    {
        ContinueNode(TokenLocation loc)
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << "Continue" << std::endl;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<ContinueNode>(Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct EqualsExpressionNode : ASTNode
    {
        NodePtr Left;
        NodePtr Right;
        bool Inverse;

        EqualsExpressionNode(NodePtr left, NodePtr right, bool inv, TokenLocation loc)
            : Left(std::move(left)), Right(std::move(right)), Inverse(inv)
        {
            this->Location = loc;
        }
        EqualsExpressionNode(const EqualsExpressionNode &other)
            : Left(other.Left ? other.Left->Clone() : nullptr),
              Right(other.Right ? other.Right->Clone() : nullptr),
              Inverse(other.Inverse)
        {
            this->Location = other.Location;
        }

        void Print(std::ostream &os) const override
        {
            Left->Print(os);
            os << (Inverse ? "!=" : "==");
            Right->Print(os);
            os << "\n";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<EqualsExpressionNode>(
                Left ? Left->Clone() : nullptr,
                Right ? Right->Clone() : nullptr,
                Inverse, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct BinaryExpressionNode : ASTNode
    {
        NodePtr Left;
        NodePtr Right;
        TokenType Operator;

        BinaryExpressionNode(TokenType op, NodePtr left, NodePtr right, TokenLocation loc)
            : Operator(op), Left(std::move(left)), Right(std::move(right))
        {
            this->Location = loc;
        }
        BinaryExpressionNode(const BinaryExpressionNode &other)
            : Operator(other.Operator),
              Left(other.Left ? other.Left->Clone() : nullptr),
              Right(other.Right ? other.Right->Clone() : nullptr)
        {
            this->Location = other.Location;
        }

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
                Right ? Right->Clone() : nullptr, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct UnaryExpressionNode : ASTNode
    {
        TokenType Operator;
        NodePtr Right;

        UnaryExpressionNode(TokenType op, NodePtr right, TokenLocation loc)
            : Operator(op), Right(std::move(right))
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << TokenTypeToString(Operator);
            os << Right << std::endl;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<UnaryExpressionNode>(
                Operator, Right->Clone(), Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ImportModuleNode : ASTNode
    {
        std::string ModuleName;

        ImportModuleNode(std::string moduleName, TokenLocation loc)
            : ModuleName(std::move(moduleName))
        {
            this->Location = loc;
        }
        ImportModuleNode(const ImportModuleNode &other) : ModuleName(other.ModuleName)
        {
            this->Location = other.Location;
        }

        void Print(std::ostream &os) const override
        {
            os << "Module " << ModuleName << "\n";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<ImportModuleNode>(ModuleName, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct AssignExpressionNode : ASTNode
    {
        NodePtr Left;
        NodePtr Right;

        AssignExpressionNode(NodePtr left, NodePtr right, TokenLocation loc)
            : Left(std::move(left)), Right(std::move(right))
        {
            this->Location = loc;
        }
        AssignExpressionNode(const AssignExpressionNode &other)
            : Left(other.Left ? other.Left->Clone() : nullptr),
              Right(other.Right ? other.Right->Clone() : nullptr)
        {
            this->Location = other.Location;
        }

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
                Right ? Right->Clone() : nullptr, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ListAccessNode : ASTNode
    {
        NodePtr Object;
        NodePtr Index;

        ListAccessNode(NodePtr obj, NodePtr index, TokenLocation loc)
            : Object(std::move(obj)), Index(std::move(index))
        {
            this->Location = loc;
        }

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
                Index ? Index->Clone() : nullptr, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct MapNode : ASTNode
    {
        std::vector<std::pair<NodePtr, NodePtr>> Elements;

        MapNode(std::vector<std::pair<NodePtr, NodePtr>> elements, TokenLocation loc)
            : Elements(std::move(elements))
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override
        {
            os << "{" << std::endl;
            for (auto &pair : Elements)
            {
                os << "(";
                if (pair.first)
                    pair.first->Print(os);
                os << ") = ";
                pair.second->Print(os);
                os << std::endl;
            }
            os << "}";
        }

        NodePtr Clone() const override
        {
            std::vector<std::pair<NodePtr, NodePtr>> clonedElements;
            for (auto &pair : Elements)
            {
                if (pair.second)
                    clonedElements.emplace_back(pair.first->Clone(), pair.second->Clone());
            }
            return std::make_unique<MapNode>(std::move(clonedElements), Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct ListNode : ASTNode
    {
        std::vector<NodePtr> Elements;
        ListNode(std::vector<NodePtr> elements, TokenLocation loc)
            : Elements(std::move(elements))
        {
            this->Location = loc;
        }

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
            return std::make_unique<ListNode>(std::move(clonedElements), Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct BooleanNode : ASTNode
    {
        bool Value;
        BooleanNode(bool val, TokenLocation loc) : Value(val)
        {
            this->Location = loc;
        }

        void Print(std::ostream &os) const override { os << (Value ? "true" : "false"); }
        NodePtr Clone() const override { return std::make_unique<BooleanNode>(Value, Location); }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct IntegerNode : ASTNode
    {
        long long Value;

        IntegerNode(TokenLocation loc)
            : Value(0)
        {
            this->Location = loc;
        }

        IntegerNode(int value, TokenLocation loc)
            : Value(value)
        {
            this->Location = loc;
        }
        IntegerNode(const IntegerNode &other) : Value(other.Value)
        {
            this->Location = other.Location;
        }

        void Print(std::ostream &os) const override
        {
            os << Value;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<IntegerNode>(Value, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct FloatNode : ASTNode
    {
        float Value;

        FloatNode(TokenLocation loc)
            : Value(0.0f)
        {
            this->Location = loc;
        }

        FloatNode(float value, TokenLocation loc)
            : Value(value)
        {
            this->Location = loc;
        }

        FloatNode(const FloatNode &other) : Value(other.Value)
        {
            this->Location = other.Location;
        }

        void Print(std::ostream &os) const override
        {
            os << Value << "f";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<FloatNode>(Value, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct StringNode : ASTNode
    {
        std::string Value;

        StringNode(TokenLocation loc)
            : Value("")
        {
            this->Location = loc;
        }

        StringNode(const std::string &value, TokenLocation loc)
            : Value(value)
        {
            this->Location = loc;
        }

        StringNode(std::string &&value, TokenLocation loc)
            : Value(std::move(value))
        {
            this->Location = loc;
        }

        StringNode(const StringNode &other) : Value(other.Value)
        {
            this->Location = other.Location;
        }

        void Print(std::ostream &os) const override
        {
            os << "\"" << Value << "\"";
        }

        NodePtr Clone() const override
        {
            return std::make_unique<StringNode>(Value, Location);
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

        IdentifierNode(const std::string &value, TokenLocation loc)
            : Value(value)
        {
            this->Location = loc;
        }

        IdentifierNode(std::string &&value, TokenLocation loc)
            : Value(std::move(value))
        {
            this->Location = loc;
        }

        IdentifierNode(const IdentifierNode &other) : Value(other.Value)
        {
            this->Location = other.Location;
        }

        void Print(std::ostream &os) const override
        {
            os << Value;
        }

        NodePtr Clone() const override
        {
            return std::make_unique<IdentifierNode>(Value, Location);
        }

        EvaluatedValue Accept(Visitor &visitor) const override;
    };

    struct FunctionObject
    {
        std::string Name;
        enum class Type
        {
            USER_DEFINED,
            BUILTIN
        };
        Type Type;

        std::shared_ptr<FunctionDefinitionNode> UserFuncNodeAst;
        SymbolTableStack CapturedEnvironment;

        FunctionObject(std::string n, std::shared_ptr<FunctionDefinitionNode> funcNode, SymbolTableStack stack)
            : Name(std::move(n)), Type(Type::USER_DEFINED), UserFuncNodeAst(std::move(funcNode)), CapturedEnvironment(std::move(stack)) {}
        explicit FunctionObject(std::string n)
            : Name(std::move(n)), Type(Type::BUILTIN), UserFuncNodeAst(nullptr) {}
    };
} // namespace Aleng