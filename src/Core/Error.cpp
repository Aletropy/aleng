#include "Error.h"

#include <iostream>
#include <sstream>

namespace Aleng
{
    void PrintFormattedError(const AlengError &err, const std::string &sourceCode)
    {
        std::cerr << "Runtime Error: " << err.what() << std::endl;

        auto [Line, Column, FilePath] = err.GetLocation();
        std::cerr << "  --> " << FilePath << ":" << Line << ":" << Column << std::endl;
        std::cerr << "    |" << std::endl;

        std::vector<std::string> lines;
        std::stringstream ss(sourceCode);
        std::string line;

        while (std::getline(ss, line))
            lines.push_back(line);

        if (Line > 0 && Line <= lines.size())
        {
            const std::string lineNumStr = std::to_string(Line);
            std::cerr << " " << lineNumStr << " | " << lines[Line - 1] << std::endl;

            std::cerr << "    | ";
            for (int i = 0; i < Column; i++)
            {
                if (lines[Line - 1][i] == '\t')
                    std::cerr << '\t';
                else
                    std::cerr << ' ';
            }
            std::cerr << "^" << std::endl;
        }
        else
        {
            std::cerr << "    | (Could not retrieve source line)" << std::endl;
        }
    }
}
