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
        explicit ModuleManager(fs::path workspaceRoot);

        void RegisterNativeLibrary(const std::string& name, NativeLibrary library);
        EvaluatedValue LoadModule(const std::string& name, const ImportModuleNode& contextNode,
            Visitor& visitor);

        void RegisterModule(const std::string& name, const MapStorage& exportsMap);

    private:
        fs::path m_WorkspaceRoot;
        std::unordered_map<std::string, EvaluatedValue> m_ModulesCache = {};
        std::unordered_map<std::string, NativeLibrary> m_NativeLibraries = {};
    };
} // Aleng