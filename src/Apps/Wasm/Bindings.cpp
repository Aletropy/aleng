#include <iostream>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <sstream>
#include <string>

#include "Core/Visitor.h"
#include "Core/Parser.h"
#include "Core/ModuleManager.h"
#include "Core/NativeRegistry.cpp"

using namespace emscripten;
using namespace Aleng;

class AlengWasmInterface {
public:
    AlengWasmInterface()
        : m_ModuleManager("/virtual_fs"),
          m_Visitor(m_ModuleManager)
    {
        RegisterAllNativeLibraries(m_ModuleManager);
    }

    std::string Execute(std::string sourceCode) {
        try {
            Parser parser(sourceCode, "playground.aleng");
            auto program = parser.ParseProgram();

            auto result = program->Accept(m_Visitor);

            return "Execution finished.";
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

private:
    ModuleManager m_ModuleManager;
    Visitor m_Visitor;
};

EMSCRIPTEN_BINDINGS(aleng_module) {
    class_<AlengWasmInterface>("Aleng")
        .constructor<>()
        .function("execute", &AlengWasmInterface::Execute);
}