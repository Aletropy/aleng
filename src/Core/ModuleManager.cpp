#include "ModuleManager.h"

#include <fstream>
#include <utility>

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

    EvaluatedValue ModuleManager::LoadModule(const std::string &name, const ImportModuleNode &contextNode, Visitor &visitor)
    {
        if (m_ModulesCache.contains(name))
        {
            return m_ModulesCache[name];
        }

        if (m_NativeLibraries.contains(name))
        {
            auto [Functions] = m_NativeLibraries.at(name);
            const auto exportsMap = std::make_shared<MapRecursiveWrapper>();

            for(const auto &[funcName, funcCallback]: Functions)
            {
                auto funcObj = std::make_shared<FunctionObject>(funcName);
                exportsMap->elements[funcName] = std::make_shared<FunctionObject>(name + "." + funcName);
            }

            auto exports = std::make_shared<MapRecursiveWrapper>();
            for (const auto& [funcName, funcCallback] : Functions)
            {
                std::string fullName = "native::" + name + "::" + funcName;
                visitor.RegisterBuiltinCallback(fullName, funcCallback);

                auto funcObject = std::make_shared<FunctionObject>(fullName);
                exports->elements[funcName] = funcObject;
            }

            m_ModulesCache[name] = exports;
            return exports;
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
            return visitor.ExecuteModule(sourceCode, contextNode, modulePath.string());
        } catch (const AlengError &err)
        {
            throw;
        }
        catch (const std::exception& e)
        {
            throw;
        }

        throw AlengError("Module '" + name + "' could not be found as a native or scripted module.", contextNode);
    }
} // Aleng