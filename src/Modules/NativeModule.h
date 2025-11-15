#pragma once

#include <functional>

#include "Core/AST.h"
#include "Core/Error.h"

namespace Aleng
{
    class Visitor;

    using BuiltinFunctionCallback = std::function<EvaluatedValue(Visitor&, const std::vector<EvaluatedValue>&, const FunctionCallNode&)>;
    using NativeFunctionMap = std::unordered_map<std::string, BuiltinFunctionCallback>;

    struct NativeLibrary
    {
        NativeFunctionMap Functions;
        std::unordered_map<std::string, EvaluatedValue> Variables;
    };

    inline bool IsTruthy(const EvaluatedValue &val)
    {
        if (const auto pval = std::get_if<double>(&val))
            return *pval != 0.0;
        else if (const auto sval = std::get_if<std::string>(&val))
            return !sval->empty();
        else if (const auto bval = std::get_if<bool>(&val))
            return *bval;
        else if (const auto lval = std::get_if<ListStorage>(&val))
            return !(*lval)->elements.empty();
        else if (const auto mval = std::get_if<MapStorage>(&val))
            return !(*mval)->elements.empty();

        return false;
    }

    inline void ExpectArgs(const FunctionCallNode& ctx, const std::vector<EvaluatedValue>& args, int count)
    {
        if (args.size() != count)
            throw AlengError("Expected " + std::to_string(count) + " arguments.", ctx);
    }

    inline double GetNumber(const FunctionCallNode& ctx, const EvaluatedValue& val, const std::string& paramName) {
        if (const auto pNum = std::get_if<double>(&val)) {
            return *pNum;
        }
        throw AlengError("Parameter '" + paramName + "' must be a Number.", ctx);
    }

    inline bool ValuesAreEqual(const EvaluatedValue& a, const EvaluatedValue& b)
    {
        bool result = false;
        if (const auto dPtr = std::get_if<double>(&a))
            result = *dPtr == std::get<double>(b);
        else if (const auto strPtr = std::get_if<std::string>(&a))
            result = *strPtr == std::get<std::string>(b);
        else if (const auto boolPtr = std::get_if<bool>(&a))
            result = *boolPtr == std::get<bool>(b);
        else if (const auto listPtr = std::get_if<ListStorage>(&a))
            result = *listPtr == std::get<ListStorage>(b);
        else if (const auto mapPtr = std::get_if<MapStorage>(&a))
            result = *mapPtr == std::get<MapStorage>(b);
        else if (const auto funcPtr = std::get_if<FunctionStorage>(&a))
            result = *funcPtr == std::get<FunctionStorage>(b);

        return result;
    }
}
