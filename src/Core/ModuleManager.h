#pragma once
#include <filesystem>

#include "AST.h"
#include "Modules/NativeModule.h"

namespace fs = std::filesystem;

namespace Aleng
{
    class ModuleManager
    {
    public:
        ModuleManager(fs::path workspaceRoot);

        void RegisterNativeLibrary(const std::string& name, NativeLibrary library);
        EvaluatedValue LoadModule(const std::string& name, const ImportModuleNode& contextNode,
            Visitor& visitor);

    private:
        fs::path m_WorkspaceRoot;
        std::unordered_map<std::string, EvaluatedValue> m_ModulesCache;
        std::unordered_map<std::string, NativeLibrary> m_NativeLibraries;
    };
} // Aleng