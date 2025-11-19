#include <cmath>

#include "NativeModule.h"
#include <numbers>

namespace Aleng::StdLib
{
    EvaluatedValue Math_Sin(Visitor& visitor, const std::vector<EvaluatedValue>& args, const FunctionCallNode& ctx)
    {
        ExpectArgs(ctx, args, 1);
        const double val = GetNumber(ctx, args[0], "angle");
        return std::sin(val);
    }

    EvaluatedValue Math_Cos(Visitor& visitor, const std::vector<EvaluatedValue>& args, const FunctionCallNode& ctx)
    {
        ExpectArgs(ctx, args, 1);
        const double val = GetNumber(ctx, args[0], "angle");
        return std::cos(val);
    }

    NativeLibrary CreateMathLibrary()
    {
        NativeLibrary lib;
        lib.Functions["Sin"] = Math_Sin;
        lib.Functions["Cos"] = Math_Cos;
        lib.Variables["PI"] = std::numbers::pi;

        return lib;
    }

}
