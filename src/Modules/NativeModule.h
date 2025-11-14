#pragma once

#include <functional>

#include "Core/AST.h"
#include "Core/Error.h"

namespace Aleng
{
    using BuiltinFunctionCallback = std::function<EvaluatedValue(Visitor&, const std::vector<EvaluatedValue>&, const FunctionCallNode&)>;
    using NativeFunctionMap = std::unordered_map<std::string, BuiltinFunctionCallback>;

    struct NativeLibrary
    {
        NativeFunctionMap Functions;
    };

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
}
