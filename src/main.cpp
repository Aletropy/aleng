#include <iostream>

#include "Core/Visitor.h"
#include "Core/Parser.h"
#include "Core/Error.h"

#include <filesystem>
#include <map>
#include <fstream>
#include <sstream>

#include "Core/ModuleManager.h"

namespace fs = std::filesystem;
using namespace Aleng;

namespace Aleng {
    void RegisterAllNativeLibraries(ModuleManager& manager);
}

void RunREPL(Visitor &visitor)
{
    while (true)
    {
        std::cout << "Aleng$ ";

        char buffer[2048];
        std::cin.getline(buffer, 2048);

        auto ss = std::stringstream(buffer);

        if (ss.str() == ".exit")
        {
            std::cout << "Exiting..." << std::endl;
            break;
        }

        try
        {
            auto parser = Parser(ss.str(), "REPL");
            auto ast = parser.ParseProgram();

            auto result = ast->Accept(visitor);
        }
        catch (const AlengError &err)
        {
            std::string sourceCode = ss.str();
            PrintFormattedError(err, sourceCode);
        }
        catch (const std::runtime_error &err)
        {
            std::cout << "FATAL ERROR: " << err.what() << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    fs::path workspacePath;
    std::string mainFilename = "main.aleng";
    fs::path resolvedMainFilePath;

    auto replModuleManager = ModuleManager(fs::current_path());
    RegisterAllNativeLibraries(replModuleManager);
    auto replVisitor = Visitor(replModuleManager);

    if (argc >= 2)
    {
        std::string argument = argv[1];

        if (argument.starts_with("--") && argument == "--repl")
        {
            RunREPL(replVisitor);
            return 0;
        }

        if (fs::path targetPath(argument); fs::is_directory(targetPath))
        {
            workspacePath = targetPath;
            if (!fs::exists(workspacePath / mainFilename))
            {
                std::cerr << "Error: " << mainFilename << " not found in " << workspacePath << std::endl;
                return 1;
            }
        }
        else if (fs::is_regular_file(targetPath))
        {
            workspacePath = targetPath.parent_path();
            mainFilename = targetPath.filename().string();
        }
        else
        {
            std::cerr << "Error: invalid path " << argument << std::endl;
        }
    }
    else
    {
        bool foundMain = false;
        try
        {
            for (const auto &entry : fs::recursive_directory_iterator(fs::current_path()))
            {
                if (entry.is_regular_file() && entry.path().filename() == mainFilename)
                {
                    resolvedMainFilePath = fs::absolute(entry.path());
                    workspacePath = resolvedMainFilePath.parent_path();
                    foundMain = true;
                    break;
                }
            }
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Filesystem error while searching for main.aleng: " << e.what() << std::endl;
            return 1;
        }

        if (!foundMain)
        {
            std::cerr << "Error: Could not find '" << mainFilename
                      << "' recursively from the current directory. Try specifying a path or '--repl'." << std::endl;
            return 1;
        }
    }

    if (workspacePath.empty()) {
        if (!resolvedMainFilePath.empty()) {
            workspacePath = resolvedMainFilePath.parent_path();
        } else {
            workspacePath = fs::current_path();
        }
    }

    auto moduleManager = ModuleManager(workspacePath);
    RegisterAllNativeLibraries(moduleManager);

    Visitor visitor(moduleManager);

    try
    {
        if (fs::exists(resolvedMainFilePath))
        {
            auto result = Visitor::ExecuteAlengFile(resolvedMainFilePath.string(), visitor);
        }
        else
        {
            std::cerr << "Error: Could not find " << resolvedMainFilePath.string() << std::endl;
            return 1;
        }
    }
    catch (const AlengError &err)
    {
        std::string sourceCode;

        if (std::string filepath = err.GetLocation().FilePath; !filepath.empty() && filepath != "REPL")
        {
            if (std::ifstream file(filepath); file)
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                sourceCode = buffer.str();
            }
        }

        PrintFormattedError(err, sourceCode);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << "FATAL: " << err.what() << std::endl;
        return 1;
    }
}