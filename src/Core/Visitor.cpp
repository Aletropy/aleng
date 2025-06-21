#include "Visitor.h"

#include "Window/Window.h"
#include <fstream>
#include <filesystem>
#include <sstream>

#include "Parser.h"

namespace fs = std::filesystem;

namespace Aleng
{
    std::string AlengTypeToString(AlengType type)
    {
        switch (type)
        {
        case AlengType::NUMBER:
            return "NUMBER";
        case AlengType::STRING:
            return "STRING";
        case AlengType::ANY:
            return "ANY";
        default:
            return "UNKNOWN_ALENG_TYPE";
        }
    }

}

namespace Aleng
{
    bool Visitor::IsTruthy(const EvaluatedValue &val)
    {
        if (auto pval = std::get_if<double>(&val))
        {
            return *pval != 0.0;
        }
        if (auto sval = std::get_if<std::string>(&val))
        {
            return !sval->empty();
        }

        return false;
    }

    AlengType Visitor::GetAlengType(const EvaluatedValue &val)
    {
        if (std::holds_alternative<double>(val))
            return AlengType::NUMBER;
        if (std::holds_alternative<std::string>(val))
            return AlengType::STRING;
        throw std::runtime_error("Unsupported EvaluatedValue type encountered in GetAlengType.");
    }

    Visitor::Visitor()
    {
        PushScope();
        RegisterBuiltinFunctions();
    }

    void Visitor::PushScope()
    {
        m_SymbolTableStack.emplace_back();
    }

    void Visitor::PopScope()
    {
        if (!m_SymbolTableStack.empty())
        {
            m_SymbolTableStack.pop_back();
        }

        if (m_SymbolTableStack.empty())
        {
            std::cerr << "Warning: Symbol table stack was empty, re-initializing global scope." << std::endl;
            PushScope();
        }
    }

    void Visitor::DefineVariable(const std::string &name, const EvaluatedValue &value, bool allowRedefinitionCurrentScope)
    {
        if (m_SymbolTableStack.empty())
            PushScope();

        if (!allowRedefinitionCurrentScope && IsVariableDefinedInCurrentScope(name))
            throw std::runtime_error("Variable '" + name + "' already defined in the current scope.");

        m_SymbolTableStack.back()[name] = value;
    }

    EvaluatedValue Visitor::LookupVariable(const std::string &name)
    {
        for (auto it = m_SymbolTableStack.rbegin(); it != m_SymbolTableStack.rend(); ++it)
            if (it->count(name))
                return it->at(name);

        throw std::runtime_error("Identifier \"" + name + "\" not defined.");
    }

    bool Visitor::IsVariableDefinedInCurrentScope(const std::string &name)
    {
        if (m_SymbolTableStack.empty())
            return false;
        return m_SymbolTableStack.back().count(name) > 0;
    }

    void Visitor::RegisterBuiltinFunctions()
    {
        m_Functions.emplace("Print", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                              {
            for (auto &arg : args)
                PrintEvaluatedValue(arg);
            return 0.0; }));

        m_Functions.emplace("IsString", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                                 {
            bool isString = true;
            for (auto &arg : args)
                if(!std::holds_alternative<std::string>(arg)) isString = false;
            return isString ? 1.0 : 0.0; }));

        m_Functions.emplace("IsNumber", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                                 {
            bool isNumber = true;
            for (auto &arg : args)
                if(!std::holds_alternative<double>(arg)) isNumber = false;
            return isNumber ? 1.0 : 0.0; }));

        m_Functions.emplace("ParseNumber", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                                    {
                                                        if (args.size() != 1)
                                                            throw std::runtime_error("ParseNumber expects exacts 1 argument");
                                                        auto& arg = args[0];
                                                        bool isNumber = std::holds_alternative<double>(arg);
                                                        return isNumber ? arg : std::stod(std::get<std::string>(arg)); }));
    }

    void Visitor::LoadModuleFiles(std::map<std::string, fs::path> moduleFiles)
    {
        m_AvailableModules = moduleFiles;
    }

    EvaluatedValue Visitor::ExecuteAlengFile(const std::string &filepath, Visitor &visitor)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Warning: Could not open file " << filepath << " for execution." << std::endl;
            return 0.0;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::string sourceCode = buffer.str();
        auto parser = Parser(sourceCode);
        auto programAst = parser.ParseProgram();

        return programAst->Accept(visitor);
    }

    EvaluatedValue Visitor::Visit(const ProgramNode &node)
    {
        EvaluatedValue latestResult;
        for (auto &nodePtr : node.Statements)
        {
            latestResult = nodePtr->Accept(*this);
        }

        return latestResult;
    }

    EvaluatedValue Visitor::Visit(const BlockNode &node)
    {
        EvaluatedValue latestResult;
        for (auto &nodePtr : node.Statements)
        {
            latestResult = nodePtr->Accept(*this);
        }

        return latestResult;
    }

    EvaluatedValue Visitor::Visit(const IfNode &node)
    {
        EvaluatedValue conditionResult = node.Condition->Accept(*this);
        if (IsTruthy(conditionResult))
        {
            return node.ThenBranch->Accept(*this);
        }
        else if (node.ElseBranch)
        {
            return node.ElseBranch->Accept(*this);
        }
        return 0.0;
    }

    EvaluatedValue Visitor::Visit(const IntegerNode &node)
    {
        return EvaluatedValue(static_cast<double>(node.Value));
    }
    EvaluatedValue Visitor::Visit(const FloatNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const StringNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const IdentifierNode &node)
    {
        for (const auto &pair : m_SymbolTableStack.back())
            if (pair.first == node.Value)
                return pair.second;

        throw std::runtime_error("Identifier \"" + node.Value + "\" not defined");
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const AssignExpressionNode &node)
    {
        auto right = node.Right->Accept(*this);

        DefineVariable(node.Left, right);

        return static_cast<EvaluatedValue>(right);
    }
    EvaluatedValue Visitor::Visit(const EqualsExpressionNode &node)
    {
        auto left = node.Left->Accept(*this);
        auto right = node.Right->Accept(*this);

        EvaluatedValue result = 0.0;

        std::visit(
            overloads{
                [&](double l, double r)
                {
                    if (l == r)
                        result = 1.0;
                    else if (l != r && node.Inverse)
                        result = 1.0;
                },
                [&](std::string l, std::string r)
                {
                    if (l == r)
                        result = 1.0;
                    else if (l != r && node.Inverse)
                        result = 1.0;
                },
                [&](auto &l, auto &r)
                {
                    result = node.Inverse ? 1.0 : 0.0;
                }},
            left, right);

        return result;
    }

    EvaluatedValue Visitor::Visit(const FunctionDefinitionNode &node)
    {
        if (m_Functions.count(node.FunctionName))
        {
            std::cout << "Warning: Redefining function '" << node.FunctionName << "'." << std::endl;
        }

        m_Functions.erase(node.FunctionName);
        m_Functions.emplace(node.FunctionName, Callable(const_cast<const FunctionDefinitionNode *>(&node)));
        return 0.0;
    }

    EvaluatedValue Visitor::Visit(const FunctionCallNode &node)
    {
        std::vector<EvaluatedValue> resolvedArgs;

        for (auto &p : node.Arguments)
            resolvedArgs.push_back(p->Accept(*this));

        auto funcIt = m_Functions.find(node.Name);
        if (funcIt == m_Functions.end())
            throw std::runtime_error("Function '" + node.Name + "' not defined.");

        const Callable &callable = funcIt->second;

        if (callable.type == Callable::Type::USER_DEFINED)
        {
            const FunctionDefinitionNode *funcDef = callable.userFuncNode;
            if (!funcDef)
                throw std::runtime_error("Internal error: User function node is null for " + node.Name);
            PushScope();

            size_t argIdx = 0;

            for (const auto &param : funcDef->Parameters)
            {
                if (argIdx >= resolvedArgs.size())
                {
                    PopScope();
                    throw std::runtime_error("Not enough arguments for function '" + funcDef->FunctionName + "'. Expected parameter '" + param.Name + "'.");
                }

                const EvaluatedValue &argVal = resolvedArgs[argIdx];
                if (param.TypeName)
                {
                    AlengType expectedType;
                    if (*param.TypeName == "NUMBER")
                        expectedType = AlengType::NUMBER;
                    else if (*param.TypeName == "STRING")
                        expectedType = AlengType::STRING;
                    else
                    {
                        PopScope();
                        throw std::runtime_error("Unknown type name '" + *param.TypeName + "' in function '" + funcDef->FunctionName + "' signature for parameter '" + param.Name + "'.");
                    }

                    AlengType actualType = GetAlengType(argVal);
                    if (actualType != expectedType)
                    {
                        PopScope();
                        throw std::runtime_error("Type mismatch for parameter '" + param.Name + "' in function '" + funcDef->FunctionName +
                                                 "'. Expected " + *param.TypeName + " (" + AlengTypeToString(expectedType) +
                                                 ") but got " + AlengTypeToString(actualType) + ".");
                    }
                }

                DefineVariable(param.Name, argVal, false);
                argIdx++;
            }

            if (argIdx < resolvedArgs.size())
            {
                PopScope();
                throw std::runtime_error("Too many arguments for function '" + funcDef->FunctionName + "'. Expected " + std::to_string(funcDef->Parameters.size()) + " arguments, got " + std::to_string(resolvedArgs.size()) + ".");
            }

            auto result = funcDef->Body->Accept(*this);
            PopScope();
            return result;
        }
        else if (callable.type == Callable::Type::BUILTIN)
            return callable.builtinFunc(*this, resolvedArgs, node);

        throw std::runtime_error("Internal error: Unknown callable type for function " + node.Name);
    }

    EvaluatedValue Visitor::Visit(const ImportModuleNode &node)
    {
        for (auto &entry : m_ImportedModules)
        {
            if (entry == node.ModuleName)
                return 0.0;
        }

        auto path = m_AvailableModules[node.ModuleName];

        if (path.empty())
            throw std::runtime_error("Module '" + node.ModuleName + "' not found.");

        m_ImportedModules.push_back(node.ModuleName);
        ExecuteAlengFile(path.string(), *this);

        return 0.0;
    }

    EvaluatedValue Visitor::Visit(const BinaryExpressionNode &node)
    {
        auto left = node.Left->Accept(*this);
        auto right = node.Right->Accept(*this);

        EvaluatedValue finalValue;

        std::visit(
            overloads{
                [&](double l, double r)
                {
                    switch (node.Operator)
                    {
                    case TokenType::PLUS:
                        finalValue = l + r;
                        break;
                    case TokenType::MINUS:
                        finalValue = l - r;
                        break;
                    case TokenType::MULTIPLY:
                        finalValue = l * r;
                        break;
                    case TokenType::DIVIDE:
                        if (r == 0.0)
                            throw std::runtime_error("Division by 0 is genious! Congratulations!");
                        finalValue = l / r;
                        break;
                    default:
                        throw std::runtime_error("Unknown operator for binary expression: " + TokenTypeToString(node.Operator));
                        break;
                    }
                },
                [&](std::string l, std::string r)
                {
                    if (node.Operator != TokenType::PLUS)
                        throw std::runtime_error("Only concatenation operator for strings supported.");
                    finalValue = l + r;
                    return;
                },
                [&](std::string l, double r)
                {
                    auto ss = std::stringstream();

                    switch (node.Operator)
                    {
                    case TokenType::PLUS:
                        ss << l;
                        ss << std::to_string(r);
                        finalValue = ss.str();
                        break;
                    case TokenType::MULTIPLY:
                        for (int i = 0; i < static_cast<int>(r); i++)
                            ss << l;
                        finalValue = ss.str();
                        break;
                    default:
                        throw std::runtime_error("Unknown operator for binary expression: " + TokenTypeToString(node.Operator));
                        break;
                    }
                },
                [&](auto &l, auto &r)
                {
                    throw std::runtime_error("Unsupported operand types for operator " + TokenTypeToString(node.Operator) +
                                             ". Left type index: " + std::to_string(left.index()) +
                                             ", Right type index: " + std::to_string(right.index()));
                }},
            left, right);
        return finalValue;
    }
}