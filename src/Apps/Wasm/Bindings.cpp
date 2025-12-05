// ReSharper disable CppExpressionWithoutSideEffects
#include <iostream>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

#include <nlohmann/json.hpp>

#include "Analyzer.h"
#include "Core/Visitor.h"
#include "Core/Parser.h"
#include "Core/ModuleManager.h"
#include "Core/NativeRegistry.cpp"

using namespace emscripten;
using namespace Aleng;
using namespace AlengLSP;

using json = nlohmann::json;

class AlengWasmInterface {
public:
    AlengWasmInterface()
        : m_ModuleManager(std::make_unique<ModuleManager>("/virtual_fs")),
          m_Visitor(std::make_unique<Visitor>(*m_ModuleManager))
    {
        RegisterAllNativeLibraries(*m_ModuleManager);
    }

    std::string Execute(std::string sourceCode) {
        try {
            Parser parser(sourceCode, "playground.aleng");
            auto program = parser.ParseProgram();

            m_ModuleManager = std::make_unique<ModuleManager>("/virtual_fs");
            RegisterAllNativeLibraries(*m_ModuleManager);
            m_Visitor = std::make_unique<Visitor>(*m_ModuleManager);

            auto result = program->Accept(*m_Visitor);
            return "";
        }
        catch (const AlengError& e) {
            std::stringstream ss;
            auto loc = e.GetRange();
            ss << "Runtime Error: " << e.what() << "\n";
            ss << "  at line " << loc.Start.Line << ", col " << loc.Start.Column;
            return ss.str();
        }
        catch (const std::exception& e) {
            return std::string("Fatal Error: ") + e.what();
        }
    }

    std::string Lint(const std::string &sourceCode)
    {
        json diagnostics = json::array();
        const std::string uri = "browser.aleng";

        try
        {
            Parser parser(sourceCode, uri);
            std::unique_ptr<ProgramNode> program = nullptr;

            try {
                program = parser.ParseProgram();
            } catch (...) {
            }

            if (parser.HasErrors())
            {
                for (const auto& err : parser.GetErrors())
                {
                    const auto loc = err.GetRange();

                    json diag;
                    diag["line"] = loc.Start.Line;
                    diag["col"] = loc.Start.Column;
                    diag["length"] = loc.End.Column - loc.Start.Column + 1;
                    diag["message"] = err.what();
                    diag["severity"] = "Error";

                    diagnostics.push_back(diag);
                }
            }

            if (program)
            {
                try
                {
                    m_Analyzer.Analyze(*program, uri);
                }
                catch (const std::exception& e)
                {
                    json diag;
                    diag["line"] = 1;
                    diag["col"] = 1;
                    diag["message"] = std::string("Analysis Error: ") + e.what();
                    diag["severity"] = "Warning";
                    diagnostics.push_back(diag);
                }
            }
        }
        catch (const std::exception& e)
        {
            json diag;
            diag["line"] = 1;
            diag["col"] = 1;
            diag["message"] = std::string("Compiler Fatal: ") + e.what();
            diag["severity"] = "Error";
            diagnostics.push_back(diag);
        }

        return diagnostics.dump();
    }

    std::string Complete(std::string sourceCode, int line, int col) {
        json items = json::array();
        const std::string uri = "browser.aleng";

        try {
            Parser parser(sourceCode, uri);
            if (auto program = parser.ParseProgram()) {
                m_Analyzer.Analyze(*program, uri);

                for (auto symbols = m_Analyzer.GetCompletions(uri, line, col); const auto& sym : symbols) {
                    json item;
                    item["label"] = sym->name;

                    int kind = 6;
                    std::string detail = "Variable";

                    switch(sym->category) {
                        case Symbol::Category::Function: kind = 3; detail = "Function"; break;
                        case Symbol::Category::Class:    kind = 7; detail = "Class"; break;
                        case Symbol::Category::Parameter: kind = 6; detail = "Parameter"; break;
                        case Symbol::Category::Property: kind = 10; detail = "Property"; break;
                        default: break;
                    }

                    if (sym->type) {
                        detail += ": " + sym->type->ToString();
                    }

                    item["kind"] = kind;
                    item["detail"] = detail;
                    item["insertText"] = sym->name;

                    if (!sym->documentation.empty()) {
                         item["documentation"] = sym->documentation;
                    }

                    items.push_back(item);
                }
            }
        } catch (...) {
        }

        std::vector<std::string> keywords = {
            "If", "Else", "For", "While", "Fn", "Return", "Break", "Continue", "Import", "True", "False"
        };

        for (const auto& kw : keywords) {
            items.push_back({
                {"label", kw},
                {"kind", 14}, // Keyword
                {"insertText", kw},
                {"detail", "Keyword"}
            });
        }

        items.push_back({
            {"label", "Fn (Snippet)"},
            {"kind", 15}, // Snippet
            {"detail", "Function Definition"},
            {"insertText", "Fn ${1:name}(${2:args})\n\t$0\nEnd"},
            {"insertTextRules", 4},
            {"documentation", "Creates a new function scope."}
        });

        items.push_back({
            {"label", "For (Snippet)"},
            {"kind", 15},
            {"insertText", "For ${1:i} = ${2:0} .. ${3:10}\n\t$0\nEnd"},
            {"insertTextRules", 4}
        });

        std::vector<std::string> builtins = {"Print", "Append", "Len", "Pop", "ToNumber"};
        for(const auto& b : builtins) {
             items.push_back({
                {"label", b},
                {"kind", 3}, // Function
                {"insertText", b + "($0)"},
                {"insertTextRules", 4},
                {"detail", "Built-in Function"}
            });
        }

        return items.dump();
    }

    std::string GetHover(const std::string &sourceCode, const int line, const int col) {
        const std::string uri = "browser.aleng";
        try {
            Parser parser(sourceCode, uri);
            if (const auto program = parser.ParseProgram()) {
                m_Analyzer.Analyze(*program, uri);
                return m_Analyzer.GetHoverInfo(uri, line, col);
            }
        } catch (...) {}
        return "";
    }

    [[nodiscard]] std::string GetSemanticTokens() const
    {
        const std::string uri = "browser.aleng";
        return m_Analyzer.GetSemanticTokens(uri).dump();
    }

private:
    std::unique_ptr<ModuleManager> m_ModuleManager;
    std::unique_ptr<Visitor> m_Visitor;

    Analyzer m_Analyzer;
};

EMSCRIPTEN_BINDINGS(aleng_module)
{
    class_<AlengWasmInterface>("Aleng")
        .constructor<>()
        .function("execute", &AlengWasmInterface::Execute)
        .function("lint", &AlengWasmInterface::Lint)
        .function("complete", &AlengWasmInterface::Complete)
        .function("getHover", &AlengWasmInterface::GetHover)
        .function("getSemanticTokens", &AlengWasmInterface::GetSemanticTokens);
}