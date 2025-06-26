#pragma once

#include "AST.h"
#include <map>
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
        FUNCTION,
        ANY
    };
    std::string AlengTypeToString(AlengType type);

    class Visitor
    {
    public:
        Visitor();
        void LoadModuleFiles(std::map<std::string, fs::path> moduleFiles);

        static EvaluatedValue ExecuteAlengFile(const std::string &filepath, Visitor &visitor);

        EvaluatedValue Visit(const ProgramNode &node);
        EvaluatedValue Visit(const BlockNode &node);
        EvaluatedValue Visit(const IfNode &node);
        EvaluatedValue Visit(const ForStatementNode &node);
        EvaluatedValue Visit(const ListNode &node);
        EvaluatedValue Visit(const BooleanNode &node);
        EvaluatedValue Visit(const IntegerNode &node);
        EvaluatedValue Visit(const FloatNode &node);
        EvaluatedValue Visit(const StringNode &node);
        EvaluatedValue Visit(const IdentifierNode &node);
        EvaluatedValue Visit(const ListAccessNode &node);
        EvaluatedValue Visit(const AssignExpressionNode &node);
        EvaluatedValue Visit(const FunctionDefinitionNode &node);
        EvaluatedValue Visit(const FunctionCallNode &node);
        EvaluatedValue Visit(const ImportModuleNode &node);
        EvaluatedValue Visit(const BinaryExpressionNode &node);
        EvaluatedValue Visit(const EqualsExpressionNode &node);

    private:
        bool IsTruthy(const EvaluatedValue &val);
        AlengType GetAlengType(const EvaluatedValue &val);

        void PushScope();
        void PopScope();

        void DefineVariable(const std::string &name, const EvaluatedValue &value, bool allowRedefinitionCurrentScope = true);
        EvaluatedValue LookupVariable(const std::string &name);
        bool IsVariableDefinedInCurrentScope(const std::string &name);

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
            Callable(BuiltinFunctionCallback func) : type(Type::BUILTIN), builtinFunc(std::move(func)) {}
        };

    private:
        SymbolTableStack m_SymbolTableStack;
        std::unordered_map<std::string, Callable> m_Functions;

        std::vector<std::string> m_ImportedModules;
        std::map<std::string, fs::path> m_AvailableModules;

        void RegisterBuiltinFunctions();
    };

    template <class... Ts>
    struct overloads : Ts...
    {
        using Ts::operator()...;
    };
}