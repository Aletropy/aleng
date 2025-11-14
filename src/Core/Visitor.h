#pragma once

#include "AST.h"
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

namespace Aleng
{
    enum class AlengType
    {
        NUMBER,
        STRING,
        BOOLEAN,
        LIST,
        MAP,
        FUNCTION,
        ANY
    };
    std::string AlengTypeToString(AlengType type);

    class Visitor
    {
    public:
        explicit Visitor(std::string workspaceRoot = "unknown");

        static EvaluatedValue ExecuteAlengFile(const std::string &filepath, Visitor &visitor);

        EvaluatedValue Visit(const ProgramNode &node);
        EvaluatedValue Visit(const BlockNode &node);
        EvaluatedValue Visit(const IfNode &node);
        EvaluatedValue Visit(const ForStatementNode &node);
        EvaluatedValue Visit(const WhileStatementNode &node);
        EvaluatedValue Visit(const ListNode &node);
        EvaluatedValue Visit(const MapNode &node);

        static EvaluatedValue Visit(const BooleanNode &node);

        static EvaluatedValue Visit(const IntegerNode &node);

        static EvaluatedValue Visit(const FloatNode &node);

        static EvaluatedValue Visit(const StringNode &node);
        EvaluatedValue Visit(const IdentifierNode &node);
        EvaluatedValue Visit(const ListAccessNode &node);
        EvaluatedValue Visit(const ReturnNode &node);

        static EvaluatedValue Visit(const BreakNode &node);

        static EvaluatedValue Visit(const ContinueNode &node);
        EvaluatedValue Visit(const AssignExpressionNode &node);
        EvaluatedValue Visit(const FunctionDefinitionNode &node);
        EvaluatedValue Visit(const FunctionCallNode &node);
        EvaluatedValue Visit(const ImportModuleNode &node);
        EvaluatedValue Visit(const BinaryExpressionNode &node);
        EvaluatedValue Visit(const UnaryExpressionNode &node);
        EvaluatedValue Visit(const EqualsExpressionNode &node);

    private:
        static bool IsTruthy(const EvaluatedValue &val);
        static AlengType GetAlengType(const EvaluatedValue &val);

        void PushScope();
        void PopScope();

        void DefineVariable(const std::string &name, const EvaluatedValue &value, bool allowRedefinitionCurrentScope = true);
        void AssignVariable(const std::string& name, const EvaluatedValue& value);
        EvaluatedValue LookupVariable(const std::string &name);
        bool IsVariableDefinedInCurrentScope(const std::string &name) const;

        using BuiltinFunctionCallback = std::function<EvaluatedValue(Visitor &, const std::vector<EvaluatedValue> &, const FunctionCallNode &)>;

        struct Callable
        {
            enum class Type
            {
                USER_DEFINED,
                BUILTIN
            };
            Type type;
            std::shared_ptr<FunctionDefinitionNode> userFuncNode;
            BuiltinFunctionCallback builtinFunc;
            SymbolTableStack definitionEnvironment;

            Callable(std::shared_ptr<FunctionDefinitionNode> node, SymbolTableStack stack) : type(Type::USER_DEFINED), userFuncNode(std::move(node)), builtinFunc(nullptr), definitionEnvironment(std::move(stack)) {}
            explicit Callable(BuiltinFunctionCallback func) : type(Type::BUILTIN), builtinFunc(std::move(func)) {}
        };

    private:
        SymbolTableStack m_SymbolTableStack;
        std::unordered_map<std::string, Callable> m_Functions;

        fs::path m_WorkspaceRoot;
        std::unordered_map<std::string, EvaluatedValue> m_ModuleCache;

        void RegisterBuiltinFunctions();
    };

    template <class... Ts>
    struct overloads : Ts...
    {
        using Ts::operator()...;
    };
}