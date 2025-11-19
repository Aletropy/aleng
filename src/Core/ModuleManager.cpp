#include "ModuleManager.h"

#include <fstream>
#include <utility>
#include <sstream>

#include "Error.h"
#include "Parser.h"
#include "Visitor.h"

namespace Aleng
{
    ModuleManager::ModuleManager(fs::path workspaceRoot)
        : m_WorkspaceRoot(std::move(workspaceRoot))
    {
    }

    void ModuleManager::RegisterNativeLibrary(const std::string &name, NativeLibrary library)
    {
        m_NativeLibraries[name] = std::move(library);
    }

    void ModuleManager::RegisterModule(const std::string &name, const MapStorage &exportsMap)
    {
        m_ModulesCache[name] = exportsMap;
    }

    EvaluatedValue ModuleManager::LoadModule(const std::string &name, const ImportModuleNode &contextNode, Visitor &visitor)
    {
        if (m_ModulesCache.contains(name))
        {
            return m_ModulesCache[name];
        }

        if (m_NativeLibraries.contains(name))
        {
            auto [Functions, Variables] = m_NativeLibraries.at(name);
            auto exportsMap = std::make_shared<MapRecursiveWrapper>();

            for (const auto& [funcName, funcCallback] : Functions)
            {
                visitor.RegisterBuiltinCallback(funcName, funcCallback);
            }

            for (const auto& [funcName, _] : Functions)
            {
                if (!funcName.starts_with("native::"))
                {
                    exportsMap->elements[funcName] = std::make_shared<FunctionObject>(funcName);
                }
            }

            for (const auto& [varName, varValue] : Variables)
            {
                exportsMap->elements[varName] = varValue;
            }

            m_ModulesCache[name] = exportsMap;
            return exportsMap;
        }


        fs::path modulePath = m_WorkspaceRoot / (name + ".aleng");
        if (!fs::exists(modulePath))
        {
            throw AlengError("Module '" + name + "' not found.", contextNode);
        }

        std::ifstream file(modulePath);
        if (!file.is_open())
        {
            throw AlengError("Failed to open module file '" + modulePath.string() + "'.", contextNode);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string sourceCode = buffer.str();
        file.close();

        try
        {
            return visitor.ExecuteAndStoreModule(sourceCode, contextNode, modulePath.string());
        } catch (const AlengError &err)
        {
            throw;
        }
        catch (const std::exception& e)
        {
            throw;
        }
    }
} // Aleng