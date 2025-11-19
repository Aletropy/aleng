#include <iostream>
#include <string>
#include <vector>

#include "Core/AST.h"
#include "Core/Error.h"
#include "Core/Visitor.h"

namespace Aleng::StdLib
{
    struct TestSuite
    {
        std::string Name;
        std::vector<std::pair<std::string, FunctionStorage>> Tests;
    };

    static std::unordered_map<double, TestSuite> g_TestSuites;
    static double g_NextSuiteId = 0;

    EvaluatedValue Test_RunSuite(Visitor& visitor, const std::vector<EvaluatedValue>& args, const FunctionCallNode& ctx, const double suiteId)
    {
        if (!g_TestSuites.contains(suiteId)) {
            throw AlengError("Internal test suite error: invalid ID.", ctx);
        }

        const auto&[Name, Tests] = g_TestSuites.at(suiteId);
        std::cout << "\n\033[1m▶ Running suite: " << Name << "\033[0m" << std::endl;

        int passed = 0;
        int failed = 0;

        for (const auto&[fst, snd] : Tests) {
            const std::string& description = fst;
            const FunctionStorage& testFunc = snd;

            auto dummyLocation = testFunc->UserFuncNodeAst ? testFunc->UserFuncNodeAst->Location : ctx.Location;
            FunctionCallNode callNode(std::make_unique<IdentifierNode>(testFunc->Name, dummyLocation), {}, dummyLocation);

            try {
                visitor.Visit(callNode);
                std::cout << "  \033[32m✔\033[0m " << description << std::endl;
                passed++;
            } catch (const AlengError& err) {
                std::cout << "  \033[31m✖\033[0m " << description << std::endl;
                std::cout << "    \033[31m[FAIL]\033[0m " << err.what() << " at " << err.GetLocation().FilePath << ":" << err.GetLocation().Line << std::endl;
                failed++;
            } catch (const std::exception& e) {
                 std::cout << "  \033[91m✖\033[0m " << description << std::endl;
                 std::cout << "    \033[91m[ERROR]\033[0m Native C++ exception: " << e.what() << std::endl;
                 failed++;
            }
        }

        std::cout << "----------" << std::endl;
        std::cout << "\033[1mSummary: " << passed << " tests passed, " << failed << " failed.\033[0m" << std::endl;

        return static_cast<double>(failed);
    }

    EvaluatedValue Test_AddTest(Visitor& visitor, const std::vector<EvaluatedValue>& args, const FunctionCallNode& ctx, const double suiteId)
    {
        ExpectArgs(ctx, args, 2);

        const auto* pDescription = std::get_if<std::string>(&args[0]);
        if (!pDescription) {
            throw AlengError("First argument to Add() must be a string description.", ctx);
        }

        const auto* pFunction = std::get_if<FunctionStorage>(&args[1]);
        if (!pFunction) {
            throw AlengError("Second argument to Add() must be a function.", ctx);
        }

        g_TestSuites[suiteId].Tests.emplace_back(*pDescription, *pFunction);

        return true;
    }

    EvaluatedValue Test_CreateSuite(Visitor& visitor, const std::vector<EvaluatedValue>& args, const FunctionCallNode& ctx)
    {
        ExpectArgs(ctx, args, 1);
        const auto* pName = std::get_if<std::string>(&args[0]);
        if (!pName) {
            throw AlengError("Suite name must be a string.", ctx);
        }

        double currentId = g_NextSuiteId++;
        g_TestSuites[currentId] = TestSuite{ *pName, {} };

        auto suiteObject = std::make_shared<MapRecursiveWrapper>();

        auto addFuncCallback = [currentId](Visitor& v, const std::vector<EvaluatedValue>& a, const FunctionCallNode& c) {
            return Test_AddTest(v, a, c, currentId);
        };
        std::string addFuncName = "native::test::suite" + std::to_string(currentId) + "::Add";
        visitor.RegisterBuiltinCallback(addFuncName, addFuncCallback);
        suiteObject->elements["Add"] = std::make_shared<FunctionObject>(addFuncName);

        auto runFuncCallback = [currentId](Visitor& v, const std::vector<EvaluatedValue>& a, const FunctionCallNode& c) {
            return Test_RunSuite(v, a, c, currentId);
        };
        std::string runFuncName = "native::test::suite" + std::to_string(currentId) + "::Run";
        visitor.RegisterBuiltinCallback(runFuncName, runFuncCallback);
        suiteObject->elements["Run"] = std::make_shared<FunctionObject>(runFuncName);

        return suiteObject;
    }

    MapStorage CreateAssertMap() {
        auto assertMap = std::make_shared<MapRecursiveWrapper>();

        assertMap->elements["Equals"] = std::make_shared<FunctionObject>("native::test::Assert::Equals");
        assertMap->elements["Throws"] = std::make_shared<FunctionObject>("native::test::Assert::Throws");
        assertMap->elements["IsTrue"] = std::make_shared<FunctionObject>("native::test::Assert::IsTrue");
        assertMap->elements["IsFalse"] = std::make_shared<FunctionObject>("native::test::Assert::IsFalse");

        return assertMap;
    }

    NativeLibrary CreateTestLibrary()
    {
        NativeLibrary lib;
        lib.Functions["CreateSuite"] = Test_CreateSuite;

        lib.Functions["native::test::Assert::Equals"] =
        [](Visitor&, const auto& args, const auto& ctx) {
            ExpectArgs(ctx, args, 3);
            if (!ValuesAreEqual(args[0], args[1])) {
                throw AlengError(std::get<std::string>(args[2]), ctx);
            }
            return true;
        };

        lib.Functions["native::test::Assert::Throws"] =
            [](Visitor& v, const auto& args, const auto& ctx) {
                ExpectArgs(ctx, args, 2);
                const auto* pFunc = std::get_if<FunctionStorage>(&args[0]);
                if (!pFunc) throw AlengError("First argument to Throws() must be a function.", ctx);

                FunctionCallNode callNode(std::make_unique<IdentifierNode>((*pFunc)->Name, ctx.Location), {}, ctx.Location);
                bool didThrow = false;
                try {
                    v.Visit(callNode);
                } catch (const AlengError&) {
                    didThrow = true;
                }
                if (!didThrow) {
                    throw AlengError(std::get<std::string>(args[1]), ctx);
                }
                return true;
        };

        lib.Functions["native::test::Assert::IsTrue"] =
        [](Visitor& v, const auto& args, const auto& ctx) {
            ExpectArgs(ctx, args, 2);
            if (!IsTruthy(args[0])) {
                throw AlengError(std::get<std::string>(args[1]), ctx);
            }
            return true;
        };

        lib.Functions["native::test::Assert::IsFalse"] =
            [](Visitor& v, const auto& args, const auto& ctx) {
                ExpectArgs(ctx, args, 2);
                if (IsTruthy(args[0])) {
                    throw AlengError(std::get<std::string>(args[1]), ctx);
                }
                return true;
        };

        lib.Variables["Assert"] = CreateAssertMap();

        return lib;
    }
}