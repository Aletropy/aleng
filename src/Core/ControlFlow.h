#pragma once

#include "AST.h"
#include <exception>

namespace Aleng
{
    class ReturnSignal : public std::exception
    {
    public:
        EvaluatedValue Value;
        ReturnSignal(EvaluatedValue val) : Value(std::move(val)) {}
    };

    class BreakSignal : public std::exception
    {
    };
    class ContinueSignal : public std::exception
    {
    };
}