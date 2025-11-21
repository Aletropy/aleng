#include <iostream>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "Analyzer.h"
#include "Core/Visitor.h"
#include "Core/Parser.h"
#include "Core/ModuleManager.h"
#include "Core/NativeRegistry.cpp"

using namespace emscripten;
using namespace Aleng;

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
            auto loc = e.GetLocation();
            ss << "Runtime Error: " << e.what() << "\n";
            ss << "  at line " << loc.Line << ", col " << loc.Column;
            return ss.str();
        }
        catch (const std::exception& e) {
            return std::string("Fatal Error: ") + e.what();
        }
    }

    std::string Lint(std::string sourceCode)
    {
        json diagnostics = json::array();

        try
        {
            Parser parser(sourceCode, "browser.aleng");
            std::unique_ptr<ProgramNode> program = nullptr;
            try
            {
                program = parser.ParseProgram();
            } catch (...)
            {

            }

            if (parser.HasErrors())
            {
                for (const auto& err : parser.GetErrors())
                {
                    const auto loc = err.GetLocation();
                    int line = std::max(1, loc.Line);
                    int col = std::max(1, loc.Column);

                    json diag;
                    diag["line"] = line;
                    diag["col"] = col;
                    diag["length"] = 5;
                    diag["message"] = err.what();
                    diag["severity"] = "Error";

                    diagnostics.push_back(diag);
                }
            } else if (program)
            {
                try
                {
                    m_Analyzer.Analyze(*program);
                } catch (const std::exception& e)
                {
                    json diag;
                    diag["line"] = 1;
                    diag["col"] = 1;
                    diag["message"] = std::string("Analysis Error: ") + e.what();
                    diag["severity"] = "Warning";
                    diagnostics.push_back(diag);
                }
            }
        } catch (const std::exception& e)
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

    std::string Complete(std::string sourceCode, int line) {
        json items = json::array();

        try {
            Parser parser(sourceCode, "browser.aleng");
            auto program = parser.ParseProgram();
            if (program) {
                m_Analyzer.Analyze(*program);

                auto symbols = m_Analyzer.GetSymbolsAt(line);

                for (const auto& sym : symbols) {
                    json item;
                    item["label"] = sym.Name;
                    // Monaco Kinds: 1=Text, 2=Method, 3=Function, 6=Variable, 14=Keyword
                    item["kind"] = (sym.Kind == "Function") ? 3 : 6;
                    item["detail"] = (sym.Kind == "Function") ? "Function defined in code" : "Variable";
                    item["insertText"] = sym.Name;
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

        // Snippet: Função
        items.push_back({
            {"label", "Fn (Snippet)"},
            {"kind", 15}, // Snippet
            {"detail", "Definição de Função"},
            {"insertText", "Fn ${1:name}(${2:args})\n\t$0\nEnd"},
            {"insertTextRules", 4}, // 4 = InsertAsSnippet
            {"documentation", "Cria um bloco de função"}
        });

        // Snippet: Loop For
        items.push_back({
            {"label", "For (Snippet)"},
            {"kind", 15},
            {"insertText", "For ${1:i} = ${2:0} .. ${3:10}\n\t$0\nEnd"},
            {"insertTextRules", 4}
        });

        // Built-ins
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

    std::string GetSemanticTokens() {
        return m_Analyzer.GetSemanticTokens().dump();
    }

private:
    std::unique_ptr<ModuleManager> m_ModuleManager;
    std::unique_ptr<Visitor> m_Visitor;
    AlengLSP::Analyzer m_Analyzer;
};

EMSCRIPTEN_BINDINGS(aleng_module) {
    // ReSharper disable once CppExpressionWithoutSideEffects
    class_<AlengWasmInterface>("Aleng")
        .constructor<>()
        .function("execute", &AlengWasmInterface::Execute)
        .function("lint", &AlengWasmInterface::Lint)
        .function("complete", &AlengWasmInterface::Complete)
        .function("getSemanticTokens", &AlengWasmInterface::GetSemanticTokens);
}