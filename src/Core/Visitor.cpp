#include "Visitor.h"

#include <cmath>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <sstream>

#include "StdLib.h"

#include "Parser.h"

#include "Error.h"

#include "ControlFlow.h"

#include "ModuleManager.h"

namespace fs = std::filesystem;

namespace Aleng
{
    std::string AlengTypeToString(AlengType type)
    {
        switch (type)
        {
        case AlengType::NUMBER:
            return "Number";
        case AlengType::STRING:
            return "String";
        case AlengType::BOOLEAN:
            return "Boolean";
        case AlengType::LIST:
            return "List";
        case AlengType::MAP:
            return "Map";
        case AlengType::FUNCTION:
            return "Function";
        case AlengType::ANY:
            return "Any";
        default:
            return "UNKNOWN_ALENG_TYPE";
        }
    }

    struct ScopedEnvironmentSwap {
        SymbolTableStack& m_TargetStack;
        SymbolTableStack m_SavedStack;

        ScopedEnvironmentSwap(SymbolTableStack& visitorStack, SymbolTableStack newEnv)
            : m_TargetStack(visitorStack), m_SavedStack(std::move(visitorStack))
        {
            m_TargetStack = std::move(newEnv);
        }

        ~ScopedEnvironmentSwap() {
            m_TargetStack = std::move(m_SavedStack);
        }
    };
}

namespace Aleng
{


    AlengType Visitor::GetAlengType(const EvaluatedValue &val)
    {
        if (std::holds_alternative<double>(val))
            return AlengType::NUMBER;
        if (std::holds_alternative<std::string>(val))
            return AlengType::STRING;
        if (std::holds_alternative<bool>(val))
            return AlengType::BOOLEAN;
        if (std::holds_alternative<ListStorage>(val))
            return AlengType::LIST;
        if (std::holds_alternative<MapStorage>(val))
            return AlengType::MAP;
        if (std::holds_alternative<FunctionStorage>(val))
            return AlengType::FUNCTION;
        throw std::runtime_error("Unsupported EvaluatedValue type encountered in GetAlengType.");
    }

    Visitor::Visitor(ModuleManager& moduleManager)
        : m_ModuleManager(moduleManager)
    {
        PushScope();

        RegisterBuiltinCallback("Print", [&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                              {
            for (auto &arg : args)
                PrintEvaluatedValue(arg);
            return 0.0; });

        RegisterBuiltinCallback("PrintRaw", [&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                                 {
            for (auto &arg : args)
                PrintEvaluatedValue(arg, true);
            return 0.0; });

        RegisterBuiltinCallback("ToNumber", [&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                                {
            if (args.size() != 1)
                throw AlengError("ToNumber expects exacts 1 argument", ctx);
            auto& arg = args[0];
            const bool isNumber = std::holds_alternative<double>(arg);
            return isNumber ? arg : std::stod(std::get<std::string>(arg)); });

        RegisterBuiltinCallback("Append", [&](Visitor &, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                               {
                if (args.size() < 2)
                    throw AlengError("Append expects (List, ...).", ctx);
                auto objectVal = args[0];

                if (auto pListWrapper = std::get_if<ListStorage>(&objectVal))
                {
                    for(size_t i = 1; i < args.size(); i++)
                        (*pListWrapper)->elements.push_back(args[i]);
                    return *pListWrapper;
                }

                throw AlengError(
                    "Object of type '" + AlengTypeToString(GetAlengType(objectVal)) + "' not supported for Append function.", ctx); });
        RegisterBuiltinCallback("Pop", [&](Visitor &, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                            {
                if (args.size() != 1)
                    throw AlengError("Len expects exacts one list as argument.", ctx);
                auto objectVal = args[0];

                if (const auto pListWrapper = std::get_if<ListStorage>(&objectVal))
                {
                    if(const auto& elements = (*pListWrapper)->elements; !elements.empty())
                    {
                        auto element = elements[elements.size()-1];
                        (*pListWrapper)->elements.pop_back();
                        return element;
                    }
                    else
                        return false;
                }

                throw AlengError(
                    "Object of type '" + AlengTypeToString(GetAlengType(objectVal)) + "' not supported for Pop function.", ctx); });


        for (auto stdLibs = StdLib::GetLibraries(); const auto& [name, source] : stdLibs)
        {
            try
            {
                Parser parser(source, name);
                const auto ast = parser.ParseProgram();

                PushScope();
                ast->Accept(*this);

                auto exportsMap = std::make_shared<MapRecursiveWrapper>();
                if (!m_SymbolTableStack.empty())
                {
                    for (const auto& [varName, value] : *m_SymbolTableStack.back())
                    {
                        exportsMap->elements[varName] = value;
                    }
                }
                PopScope();

                m_ModuleManager.RegisterModule(name, exportsMap);
            }
            catch (const AlengError& _)
            {
                throw;
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error(
                    "Fatal Error: Could not initialize standard library '" + name + "': " + e.what());
            }
        }
    }

    void Visitor::PushScope()
    {
        m_SymbolTableStack.push_back(std::make_shared<SymbolTable>());
    }

    void Visitor::PopScope()
    {
        if (!m_SymbolTableStack.empty())
        {
            m_SymbolTableStack.pop_back();
        }

        if (m_SymbolTableStack.empty())
            throw std::runtime_error("Symbol table stack was empty, no global scope.");
    }

    void Visitor::DefineVariable(const std::string &name, const EvaluatedValue &value, bool allowRedefinitionCurrentScope)
    {
        if (m_SymbolTableStack.empty())
            PushScope();

        if (!allowRedefinitionCurrentScope && IsVariableDefinedInCurrentScope(name))
            throw std::runtime_error("Variable '" + name + "' already defined in the current scope.");

        (*m_SymbolTableStack.back())[name] = value;
    }

    void Visitor::AssignVariable(const std::string &name, const EvaluatedValue &value)
    {
        for (const auto & scope_ptr : std::ranges::reverse_view(m_SymbolTableStack))
        {
            if (scope_ptr->contains(name))
            {
                (*scope_ptr)[name] = value;
                return;
            }
        }

        (*m_SymbolTableStack.back())[name] = value;
    }

    EvaluatedValue Visitor::LookupVariable(const std::string &name)
    {
        for (const auto & scope_ptr : std::ranges::reverse_view(m_SymbolTableStack))
            if (scope_ptr->contains(name))
                return scope_ptr->at(name);

        throw std::runtime_error("Identifier \"" + name + "\" not defined.");
    }

    bool Visitor::IsVariableDefinedInCurrentScope(const std::string &name) const
    {
        if (m_SymbolTableStack.empty())
            return false;
        return m_SymbolTableStack.back()->contains(name);
    }

    /*void Visitor::RegisterBuiltinFunctions()
    {





        m_Functions.emplace("Len", Callable([&](Visitor &, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                            {
                if (args.size() != 1)
                    throw AlengError("Len expects exacts one argument.", ctx);
                auto objectVal = args[0];

                if (const auto pString = std::get_if<std::string>(&objectVal))
                    return static_cast<double>(pString->length());
                else if (const auto pListWrapper = std::get_if<ListStorage>(&objectVal))
                    return static_cast<double>((*pListWrapper)->elements.size());

                throw AlengError(
                    "Object of type '" + AlengTypeToString(GetAlengType(objectVal)) + "' not supported for Len function.", ctx); }));

        m_Functions.emplace("Append", Callable([&](Visitor &, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                               {
                if (args.size() < 2)
                    throw AlengError("Append expects (List, ...).", ctx);
                auto objectVal = args[0];

                if (auto pListWrapper = std::get_if<ListStorage>(&objectVal))
                {
                    for(size_t i = 1; i < args.size(); i++)
                        (*pListWrapper)->elements.push_back(args[i]);
                    return *pListWrapper;
                }

                throw AlengError(
                    "Object of type '" + AlengTypeToString(GetAlengType(objectVal)) + "' not supported for Append function.", ctx); }));
        m_Functions.emplace("Pop", Callable([&](Visitor &, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                            {
                if (args.size() != 1)
                    throw AlengError("Len expects exacts one list as argument.", ctx);
                auto objectVal = args[0];

                if (const auto pListWrapper = std::get_if<ListStorage>(&objectVal))
                {
                    if(const auto& elements = (*pListWrapper)->elements; !elements.empty())
                    {
                        auto element = elements[elements.size()-1];
                        (*pListWrapper)->elements.pop_back();
                        return element;
                    }
                    else
                        return false;
                }

                throw AlengError(
                    "Object of type '" + AlengTypeToString(GetAlengType(objectVal)) + "' not supported for Pop function.", ctx); }));

        m_Functions.emplace("Assert", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                               {
                        if (args.size() != 2)
                            throw AlengError("Assert syntax incorrect. Expected: Assert(condition : Boolean, errorMessage : String)", ctx);

                        auto conditionVal = args[0];
                        auto messageVal = args[1];

                        if (auto pCondition = std::get_if<bool>(&conditionVal))
                        {
                            if(auto pMessageStr = std::get_if<std::string>(&messageVal))
                            {
                                if(!(*pCondition))
                                    throw AlengError(*pMessageStr, ctx);
                                return true;
                            }
                            else throw AlengError("Message must be an String.", ctx);
                        }
                        else
                            throw AlengError("Condition must be an Boolean.", ctx); }));

        m_Functions.emplace("Error", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                              {
            if (args.size() != 1)
                throw AlengError("Error syntax incorrect. Expected: Error(errorMessage : String) -> Void", ctx);

            auto messageVal = args[0];

            if (auto pMessageStr = std::get_if<std::string>(&messageVal))
                throw AlengError(*pMessageStr, ctx);
            else
                throw AlengError("Message must be an String.", ctx); }));

        m_Functions.emplace("ReadFile", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                                 {
            if (args.size() != 1)
                throw AlengError("ReadFile syntax incorrect. Expected ReadFile(path : String) -> String", ctx);

            auto filePathVal = args[0];

            if (auto pFilepath = std::get_if<std::string>(&filePathVal))
            {
                auto filepath = fs::absolute(fs::path(*pFilepath));

                if (!fs::exists(filepath))
                    throw AlengError("Path '" + filepath.string() + "' not exists.", ctx);

                if (fs::is_directory(filepath))
                    throw AlengError("Path '" + filepath.string() + "' is a directory, not an file.", ctx);

                std::ifstream file(filepath);

                if (!file.is_open())
                    throw AlengError("Path '" + filepath.string() + "' could not be opened.", ctx);

                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();

                return buffer.str();
            }
            else
                throw AlengError("'path' must be an String.", ctx); }));

        m_Functions.emplace("Exit", Callable([&](Visitor &visitor, const std::vector<EvaluatedValue> &args, const FunctionCallNode &ctx) -> EvaluatedValue
                                             {
            if(args.size() > 1)
                throw AlengError("Exit syntax error. Expected: Error(code : Number?)", ctx);

            EvaluatedValue code = 0.0;

            if(!args.empty())
                code = args[0];

            if(const auto pCodeDouble = std::get_if<double>(&code))
            {
                const auto status = static_cast<int>(*pCodeDouble);
                exit(status);
            } else throw AlengError("'code' must be a number.", ctx); }));

    }*/

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
        auto parser = Parser(sourceCode, filepath);
        auto programAst = parser.ParseProgram();

        return programAst->Accept(visitor);
    }

    void Visitor::RegisterBuiltinCallback(const std::string &name, Aleng::BuiltinFunctionCallback callback)
    {
        m_NativeCallbacks[name] = std::move(callback);
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

    EvaluatedValue Visitor::Visit(const ForStatementNode &node)
    {
        EvaluatedValue lastResult = 0.0;
        PushScope();

        if (node.Type == ForStatementNode::LoopType::NUMERIC && node.NumericLoopInfo)
        {
            const auto &info = *node.NumericLoopInfo;
            auto startVal = info.StartExpression->Accept(*this);
            auto endVal = info.EndExpression->Accept(*this);
            EvaluatedValue stepValRaw;
            int step = 1;

            if (info.StepExpression)
            {
                stepValRaw = info.StepExpression->Accept(*this);
                if (auto pStep = std::get_if<double>(&stepValRaw))
                    step = static_cast<int>(*pStep);
                else
                {
                    PopScope();
                    throw AlengError("Step value in For loop must be a number.", node);
                }
            }

            if (auto pStart = std::get_if<double>(&startVal))
            {
                if (auto pEnd = std::get_if<double>(&endVal))
                {
                    int current = static_cast<int>(*pStart);
                    double limit = *pEnd;

                    if (step == 0)
                    {
                        PopScope();
                        throw AlengError("Step value in For loop cannot be zero.", node);
                    }

                    if (!info.StepExpression && current > limit)
                    {
                        step = -1.0;
                    }

                    auto loopCondition = [&](double curr)
                    {
                        if (step > 0)
                        {
                            return info.IsUntil ? (curr < limit) : (curr <= limit);
                        }
                        else
                        {
                            return info.IsUntil ? (curr > limit) : (curr >= limit);
                        }
                    };

                    for (; loopCondition(current); current += step)
                    {
                        DefineVariable(info.IteratorVariableName, static_cast<double>(current));
                        try
                        {
                            lastResult = node.Body->Accept(*this);
                        }
                        catch (const ContinueSignal &_)
                        {
                        }
                        catch (const BreakSignal &_)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    PopScope();
                    throw AlengError("End value in numeric For loop must be a number.", node);
                }
            }
            else
            {
                PopScope();
                throw AlengError("Start value in numeric For loop must be a number.", node);
            }
        }
        else if (node.Type == ForStatementNode::LoopType::COLLECTION && node.CollectionLoopInfo)
        {
            const auto &info = *node.CollectionLoopInfo;

            EvaluatedValue collection = info.CollectionExpression->Accept(*this);
            if (auto pList = std::get_if<ListStorage>(&collection))
            {
                for (const auto &item : (*pList)->elements)
                {
                    DefineVariable(info.IteratorVariableName, item);
                    lastResult = node.Body->Accept(*this);
                }
            }
            else if (auto pMap = std::get_if<MapStorage>(&collection))
            {
                for (const auto &key: (*pMap)->elements | std::views::keys)
                {
                    DefineVariable(info.IteratorVariableName, key);
                    try
                    {
                        lastResult = node.Body->Accept(*this);
                    }
                    catch (const ContinueSignal &_)
                    {
                    }
                    catch (const BreakSignal &_)
                    {
                        break;
                    }
                }
            }
            else
            {
                PopScope();
                throw AlengError("For loop collection must be a List (Maps not supported yet).", node);
            }
        }
        else
        {
            PopScope();
            throw AlengError("Invalid ForStatementNode encountered during visitation.", node);
        }

        PopScope();
        return lastResult;
    }

    EvaluatedValue Visitor::Visit(const WhileStatementNode &node)
    {
        EvaluatedValue lastResult = 0.0;
        PushScope();

        while (true)
        {
            auto conditionResult = node.Condition->Accept(*this);
            if (!IsTruthy(conditionResult))
            {
                break;
            }

            try
            {
                lastResult = node.Body->Accept(*this);
            }
            catch (const ContinueSignal &_)
            {
                continue;
            }
            catch (const BreakSignal &_)
            {
                break;
            }
            catch (const ReturnSignal &signal)
            {
                PopScope();
                return signal.Value;
            }
            catch (const AlengError &_)
            {
                PopScope();
                throw;
            }
        }

        PopScope();
        return lastResult;
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

    EvaluatedValue Visitor::Visit(const ListNode &node)
    {
        auto listWrapper = std::make_shared<ListRecursiveWrapper>();
        for (const auto &elemNode : node.Elements)
        {
            listWrapper->elements.push_back(elemNode->Accept(*this));
        }
        return listWrapper;
    }

    EvaluatedValue Visitor::Visit(const MapNode &node)
    {
        auto mapWrapper = std::make_shared<MapRecursiveWrapper>();

        for (const auto &pair : node.Elements)
        {
            EvaluatedValue keyVal = pair.first->Accept(*this);

            if (auto pKeyStr = std::get_if<std::string>(&keyVal))
            {
                EvaluatedValue valueVal = pair.second->Accept(*this);
                mapWrapper->elements[*pKeyStr] = valueVal;
            }
            else
                throw AlengError("Map key must be evaluated to a string.", *pair.first);
        }

        return mapWrapper;
    }

    EvaluatedValue Visitor::ExecuteAndStoreModule(const std::string &sourceCode, const ImportModuleNode& node, const std::string& modulePath)
    {
        Parser parser(sourceCode, modulePath);
        const auto ast = parser.ParseProgram();

        PushScope();
        try
        {
            ast->Accept(*this);
        } catch (const AlengError &_)
        {
            PopScope();
            throw;
        }
        catch (const std::exception& e)
        {
            PopScope();
            throw AlengError("Internal error: " + std::string(e.what()), node);
        }

        auto exportsMap = std::make_shared<MapRecursiveWrapper>();
        if (!m_SymbolTableStack.empty())
        {
            for (const auto& [Name, Value] : (*m_SymbolTableStack.back()))
            {
                exportsMap->elements[Name] = Value;
            }
        }
        PopScope();

        const EvaluatedValue moduleExports = exportsMap;
        m_ModuleManager.RegisterModule(node.ModuleName, exportsMap);
        return moduleExports;
    }

    EvaluatedValue Visitor::Visit(const BooleanNode &node)
    {
        return static_cast<EvaluatedValue>(node.Value);
    }
    EvaluatedValue Visitor::Visit(const IntegerNode &node)
    {
        return { static_cast<double>(node.Value) };
    }
    EvaluatedValue Visitor::Visit(const FloatNode &node)
    {
        return node.Value;
    }
    EvaluatedValue Visitor::Visit(const StringNode &node)
    {
        return node.Value;
    }
    EvaluatedValue Visitor::Visit(const IdentifierNode &node)
    {
        for (const auto & scope_ptr : std::ranges::reverse_view(m_SymbolTableStack))
        {
            if (scope_ptr->contains(node.Value))
                return scope_ptr->at(node.Value);
        }

        if (m_NativeCallbacks.contains(node.Value))
        {
            return std::make_shared<FunctionObject>(node.Value);
        }

        throw AlengError("Identifier \"" + node.Value + "\" not defined as variable or function.", node);
    }
    EvaluatedValue Visitor::Visit(const ListAccessNode &node)
    {
        auto listObjectVal = node.Object->Accept(*this);
        auto indexVal = node.Index->Accept(*this);

        if (auto listWrapperPtr = std::get_if<ListStorage>(&listObjectVal))
        {
            if (auto indexDouble = std::get_if<double>(&indexVal))
            {
                int idx = static_cast<int>(*indexDouble);
                auto &listElements = (*listWrapperPtr)->elements;

                if (idx < 0 || idx >= listElements.size())
                    throw AlengError("List index " + std::to_string(idx) + " out of bounds for list of size " + std::to_string(listElements.size()), node);

                return listElements[idx];
            }
            else
                throw AlengError("List index must be a number.", node);
        }
        else if (auto mapWrapperPtr = std::get_if<MapStorage>(&listObjectVal))
        {
            if (auto pIndexStr = std::get_if<std::string>(&indexVal))
            {
                auto &mapElements = (*mapWrapperPtr)->elements;
                auto it = mapElements.find(*pIndexStr);
                if (it == mapElements.end())
                    throw AlengError("Key \"" + *pIndexStr + "\" not found in map.", *node.Index);
                return it->second;
            }
            else
                throw AlengError("Map key must be a string.", *node.Index);
        }
        std::string objectName = "Object";
        if (auto objIdNode = dynamic_cast<const IdentifierNode *>(node.Object.get()))
            objectName = "'" + objIdNode->Value + "'";
        throw AlengError(objectName + " is not an iterator, cannot perform indexed access.", node);
    }
    EvaluatedValue Visitor::Visit(const ReturnNode &node)
    {
        EvaluatedValue resultVal = node.ReturnValueExpression->Accept(*this);
        throw ReturnSignal(resultVal);
    }
    EvaluatedValue Visitor::Visit(const BreakNode &node)
    {
        throw BreakSignal();
    }
    EvaluatedValue Visitor::Visit(const ContinueNode &node)
    {
        throw ContinueSignal();
    }
    EvaluatedValue Visitor::Visit(const AssignExpressionNode &node)
    {
        auto valueToAssign = node.Right->Accept(*this);

        if (auto idNode = dynamic_cast<const IdentifierNode *>(node.Left.get()))
        {
            AssignVariable(idNode->Value, valueToAssign);
            return valueToAssign;
        }
        if (auto listAccess = dynamic_cast<const ListAccessNode *>(node.Left.get()))
        {
            auto listObjectVal = listAccess->Object->Accept(*this);
            auto indexVal = listAccess->Index->Accept(*this);

            if (auto listWrapperPtr = std::get_if<ListStorage>(&listObjectVal))
            {
                if (auto indexDouble = std::get_if<double>(&indexVal))
                {
                    int idx = static_cast<int>(*indexDouble);
                    auto &listElements = (*listWrapperPtr)->elements;
                    if (idx < 0 || idx >= listElements.size())
                        throw AlengError("List index " + std::to_string(idx) + " out of bounds for list of size " + std::to_string(listElements.size()), node);
                    listElements[idx] = valueToAssign;
                    return valueToAssign;
                }
                else
                    throw AlengError("List index must be a number.", node);
            }
            else if (auto mapWrapperPtr = std::get_if<MapStorage>(&listObjectVal))
            {
                if (auto pIndexStr = std::get_if<std::string>(&indexVal))
                {
                    (*mapWrapperPtr)->elements[*pIndexStr] = valueToAssign;
                    return valueToAssign;
                }
                else
                    throw AlengError("Map key for assignment must be a string.", *listAccess->Index);
            }
            else
            {
                std::string objectName = "Object";
                if (auto objIdNode = dynamic_cast<const IdentifierNode *>(listAccess->Object.get()))
                    objectName = "'" + objIdNode->Value + "'";
                throw AlengError(objectName + " is not a iterator, cannot perform indexed assignment.", node);
            }
        }
        if (auto memberAccess = dynamic_cast<const MemberAccessNode *>(node.Left.get()))
        {
            auto objectVal = memberAccess->Object->Accept(*this);
            const std::string& memberName = memberAccess->MemberIdentifier.Value;

            if (auto mapWrapperPtr = std::get_if<MapStorage>(&objectVal))
            {
                (*mapWrapperPtr)->elements[memberName] = valueToAssign;
                return valueToAssign;
            }

            std::string objectTypeName = AlengTypeToString(GetAlengType(objectVal));
            throw AlengError("Cannot assign to a member of a non-map type ('" + objectTypeName + "').", *memberAccess->Object);
        }

        throw AlengError("Invalid left-hand side in assignment.", node);
    }

    EvaluatedValue Visitor::Visit(const MemberAccessNode &node)
    {
        const auto objectVal = node.Object->Accept(*this);
        const std::string& memberName = node.MemberIdentifier.Value;

        if (const auto mapWrapperPtr = std::get_if<MapStorage>(&objectVal))
        {
            if (memberName == "length")
            {
                return static_cast<double>((*mapWrapperPtr)->elements.size());
            }

            auto &mapElements = (*mapWrapperPtr)->elements;
            const auto it = mapElements.find(memberName);
            if (it == mapElements.end())
                throw AlengError("Member \"" + memberName + "\" not found in map.", node);
            return it->second;
        }

        if (const auto listWrapperPtr = std::get_if<ListStorage>(&objectVal))
        {
            if (memberName == "length")
            {
                return static_cast<double>((*listWrapperPtr)->elements.size());
            }
        }

        if (const auto strPtr = std::get_if<std::string>(&objectVal))
        {
            if (memberName == "length")
            {
                return static_cast<double>(strPtr->size());
            }

            throw AlengError("Member \"" + memberName + "\" not found in string.", *node.Object);
        }

        const std::string objectTypeName = AlengTypeToString(GetAlengType(objectVal));
        throw AlengError("Member access operator '.' cannot be used on type '" + objectTypeName + "'.", *node.Object);
    }

    EvaluatedValue Visitor::Visit(const EqualsExpressionNode &node)
    {
        auto left = node.Left->Accept(*this);
        auto right = node.Right->Accept(*this);

        bool areEqual = false;

        std::visit(overloads{[&](double l, double r)
                             {
                                 areEqual = l == r;
                             },
                             [&](std::string l, std::string r)
                             {
                                 areEqual = l == r;
                             },
                             [&](bool l, bool r)
                             {
                                 areEqual = l == r;
                             },
                             [&](MapStorage l, MapStorage r)
                             {
                                 areEqual = (l->elements == r->elements);
                             },
                             [&](ListStorage l, ListStorage r)
                             {
                                 areEqual = (l->elements == r->elements);
                             },
                             [&](const FunctionStorage &l, const FunctionStorage &r)
                             {
                                 areEqual = l->Name == r->Name;
                             },
                             [&](auto &l, auto &r)
                             {
                                 throw AlengError("Invalid types for equality comparison.", node);
                             }},
                   left, right);

        if (node.Inverse)
            return !areEqual;
        else
            return areEqual;
    }

    EvaluatedValue Visitor::Visit(const FunctionDefinitionNode &node)
    {
        std::string internalName = node.FunctionName.value_or("lambda@" + std::to_string(node.Location.Line));

        auto funcNodeCopy = std::make_shared<FunctionDefinitionNode>(node);
        SymbolTableStack currentEnv = m_SymbolTableStack;

        auto functionStorage = std::make_shared<FunctionObject>(internalName, funcNodeCopy, currentEnv);

        if (node.FunctionName)
        {
            if (IsVariableDefinedInCurrentScope(*node.FunctionName))
            {
                throw AlengError("Identifier '" + *node.FunctionName + "' already defined in this scope.", node);
            }
            (*m_SymbolTableStack.back())[*node.FunctionName] = functionStorage;
        }

        return functionStorage;
    }

    EvaluatedValue Visitor::Visit(const FunctionCallNode &node)
    {
        EvaluatedValue callableVar = node.CallableExpression->Accept(*this);
        auto pFuncObj = std::get_if<FunctionStorage>(&callableVar);

        if (!pFuncObj)
        {
            std::stringstream ss;
            node.CallableExpression->Print(ss);
            throw AlengError("Expression '" + ss.str() + "' is not callable.", node);
        }

        const FunctionObject &funcObj = *pFuncObj->get();

        std::vector<EvaluatedValue> resolvedArgs;

        for (auto &p : node.Arguments)
            resolvedArgs.push_back(p->Accept(*this));

        if (funcObj.Type == FunctionObject::Type::USER_DEFINED)
        {
            if (!funcObj.UserFuncNodeAst)
                throw AlengError("Internal error: User-defined FunctionObject has no AST node for '" + funcObj.Name + "'.", node);

            ScopedEnvironmentSwap swapGuard(m_SymbolTableStack, funcObj.CapturedEnvironment);

            PushScope();

            const auto &funcDef = *funcObj.UserFuncNodeAst;

            size_t argIdx = 0;
            bool variadicProcessed = false;

            auto funcName = funcDef.FunctionName.value_or("lambda@" + std::to_string(funcDef.Location.Line));

            for (const auto &param : funcDef.Parameters)
            {
                if (param.IsVariadic)
                {
                    auto variadicList = std::make_shared<ListRecursiveWrapper>();
                    for (size_t i = argIdx; i < resolvedArgs.size(); ++i)
                    {
                        variadicList->elements.push_back(resolvedArgs[i]);
                    }
                    DefineVariable(param.Name, variadicList, false);
                    variadicProcessed = true;
                    break;
                }

                if (argIdx >= resolvedArgs.size())
                {
                    PopScope();
                    throw AlengError("Not enough arguments for function '" + funcName + "'. Expected parameter '" + param.Name + "'.", node);
                }

                const EvaluatedValue &argVal = resolvedArgs[argIdx];
                if (param.TypeName)
                {
                    AlengType expectedType;
                    if (*param.TypeName == "Number")
                        expectedType = AlengType::NUMBER;
                    else if (*param.TypeName == "String")
                        expectedType = AlengType::STRING;
                    else if (*param.TypeName == "Any")
                        expectedType = AlengType::ANY;
                    else
                    {
                        PopScope();
                        throw AlengError("Unknown type name '" + *param.TypeName + "' in function '" + funcName + "' signature for parameter '" + param.Name + "'.", node);
                    }

                    AlengType actualType = GetAlengType(argVal);
                    if (actualType != expectedType)
                    {
                        PopScope();
                        throw AlengError("Type mismatch for parameter '" + param.Name + "' in function '" + funcName +
                                             "'. Expected " + *param.TypeName + " (" + AlengTypeToString(expectedType) +
                                             ") but got " + AlengTypeToString(actualType) + ".",
                                         node);
                    }
                }

                DefineVariable(param.Name, argVal, false);
                argIdx++;
            }

            if (!variadicProcessed && argIdx < resolvedArgs.size())
            {
                PopScope();
                throw AlengError("Too many arguments for function '" + funcName + "'. Expected " + std::to_string(funcDef.Parameters.size()) + " arguments, got " + std::to_string(resolvedArgs.size()) + ".", node);
            }

            EvaluatedValue result;

            try
            {
                funcDef.Body->Accept(*this);
            }
            catch (const ReturnSignal &signal)
            {
                result = signal.Value;
            }
            catch (...)
            {
                std::exception_ptr p = std::current_exception();
                try {
                    if (p) std::rethrow_exception(p);
                }
                catch (const ReturnSignal &signal) {
                    result = signal.Value;
                }
                catch (const std::exception& e) {
                    throw;
                }
                catch (...) {
                    throw AlengError("Critical: Unknown exception thrown inside function.", node);
                }
            }

            PopScope();

            return result;
        }
        if (funcObj.Type == FunctionObject::Type::BUILTIN)
        {
            auto it = m_NativeCallbacks.find(funcObj.Name);
            if (it == m_NativeCallbacks.end())
            {
                throw AlengError("Internal error: Built-in function '" + funcObj.Name + "' not found.", node);
            }
            return it->second(*this, resolvedArgs, node);
        }

        throw AlengError("Internal error: Unknown FunctionObject type.", node);
    }

    EvaluatedValue Visitor::Visit(const ImportModuleNode &node)
    {
        return m_ModuleManager.LoadModule(node.ModuleName, node, *this);
    }

    EvaluatedValue Visitor::Visit(const BinaryExpressionNode &node)
    {
        if (node.Operator == TokenType::AND)
        {
            if (!IsTruthy(node.Left->Accept(*this)))
                return false;
            return IsTruthy(node.Right->Accept(*this));
        }
        else if (node.Operator == TokenType::OR)
        {
            if (IsTruthy(node.Left->Accept(*this)))
                return true;
            return IsTruthy(node.Right->Accept(*this));
        }

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
                        finalValue = EvaluatedValue(l + r);
                        break;
                    case TokenType::MINUS:
                        finalValue = EvaluatedValue(l - r);
                        break;
                    case TokenType::MULTIPLY:
                        finalValue = EvaluatedValue(l * r);
                        break;
                    case TokenType::DIVIDE:
                        if (r == 0.0)
                            throw AlengError("Division by 0  is an error.", node);
                        finalValue = EvaluatedValue(l / r);
                        break;
                    case TokenType::MODULO:
                        if (r == 0.0)
                            throw AlengError("Modulo by 0  is an error.", node);
                        finalValue = EvaluatedValue(std::fmod(l, r));
                        break;
                    case TokenType::GREATER:
                        finalValue = EvaluatedValue(l > r);
                        break;
                    case TokenType::GREATER_EQUAL:
                        finalValue = EvaluatedValue(l >= r);
                        break;
                    case TokenType::MINOR:
                        finalValue = EvaluatedValue(l < r);
                        break;
                    case TokenType::MINOR_EQUAL:
                        finalValue = EvaluatedValue(l <= r);
                        break;
                    default:
                        throw AlengError("Unknown operator for binary expression: " + TokenTypeToString(node.Operator), node);
                    }
                },
                [&](std::string l, std::string r)
                {
                    switch (node.Operator)
                    {
                    case TokenType::PLUS:
                        finalValue = EvaluatedValue(l + r);
                            break;
                    case TokenType::GREATER:
                        finalValue = EvaluatedValue(l > r);
                        break;
                    case TokenType::GREATER_EQUAL:
                        finalValue = EvaluatedValue(l >= r);
                        break;
                    case TokenType::MINOR:
                        finalValue = EvaluatedValue(l < r);
                        break;
                    case TokenType::MINOR_EQUAL:
                        finalValue = EvaluatedValue(l <= r);
                        break;
                    default:
                        throw AlengError("Only concatenation operator for strings supported.", node);
                    }
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
                        throw AlengError("Unknown operator for binary expression: " + TokenTypeToString(node.Operator), node);
                    }
                },
                [&](ListStorage l, ListStorage r)
                {
                    auto finalList = std::make_shared<ListRecursiveWrapper>();

                    for (const auto& elem : l->elements)
                    {
                        finalList->elements.push_back(elem);
                    }
                    for (const auto& elem : r->elements)
                    {
                        finalList->elements.push_back(elem);
                    }

                    finalValue = EvaluatedValue(finalList);
                },
                [&](auto &l, auto &r)
                {
                    throw AlengError("Unsupported operand types for operator " + TokenTypeToString(node.Operator) +
                                         ". Left type: " + AlengTypeToString(GetAlengType(left)) +
                                         ", Right type: " + AlengTypeToString(GetAlengType(right)),
                                     node);
                }},
            left, right);

        return finalValue;
    }

    EvaluatedValue Visitor::Visit(const UnaryExpressionNode &node)
    {
        EvaluatedValue right = node.Right->Accept(*this);
        if (node.Operator == TokenType::NOT)
        {
            return !IsTruthy(right);
        }

        throw AlengError("Unsupported unary operator '" + TokenTypeToString(node.Operator) + "'.", node);
    }
}