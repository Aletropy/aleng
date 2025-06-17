#include <iostream>
#include "Core/Visitor.h"
#include "Core/Parser.h"

#include <filesystem>
#include <map>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
using namespace Aleng;

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
            auto parser = Parser(ss.str());
            auto ast = parser.ParseProgram();

            auto result = ast->Accept(visitor);

            if (auto p = std::get_if<double>(&result))
                std::cout << *p << std::endl;
            else if (auto p = std::get_if<std::string>(&result))
                std::cout << *p << std::endl;
        }
        catch (std::runtime_error err)
        {
            std::cout << "Error: " << err.what() << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    auto visitor = Visitor();

    fs::path workspacePath;
    std::string mainFilename = "main.aleng";
    fs::path resolvedMainFilePath;

    if (argc >= 2)
    {
        std::string argument = argv[1];

        if (argument.starts_with("--") && argument == "--repl")
        {
            RunREPL(visitor);
            return 0;
        }
        else
        {
            fs::path targetPath(argument);

            if (fs::is_directory(targetPath))
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

    if (workspacePath.empty() && !resolvedMainFilePath.empty())
    {
        workspacePath = resolvedMainFilePath.parent_path();
    }

    std::map<std::string, fs::path> alengFiles;

    try
    {
        for (const auto &entry : fs::recursive_directory_iterator(workspacePath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".aleng")
            {
                if (entry.path().filename().string() != mainFilename)
                {
                    alengFiles[entry.path().stem()] = (entry.path());
                }
            }
        }

        visitor.LoadModuleFiles(alengFiles);

        if (fs::exists(resolvedMainFilePath))
        {
            auto result = Visitor::ExecuteAlengFile(resolvedMainFilePath.string(), visitor);

            // if (auto d = std::get_if<double>(&result))
            //     std::cout << *d << std::endl;
            // if (auto s = std::get_if<std::string>(&result))
            //     std::cout << *s << std::endl;
        }
        else
        {
            std::cerr << "Error: Could not find " << resolvedMainFilePath.string() << std::endl;
            return 1;
        }
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << "Runtime Error: " << err.what() << std::endl;
        return 1;
    }
}