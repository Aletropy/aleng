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
        const char* what() const noexcept override {
            return "ReturnSignal (Control Flow)";
        }
    };

    class BreakSignal : public std::exception
    {
    };
    class ContinueSignal : public std::exception
    {
    };
}