#include "Error.h"

#include <iostream>
#include <sstream>

namespace Aleng
{
    void PrintFormattedError(const AlengError &err, const std::string &sourceCode = "")
    {
        std::cerr << "Runtime Error: " << err.what() << std::endl;

        auto [Line, Column] = err.GetRange().Start;
        const auto FilePath = err.GetRange().FilePath;
        std::cerr << "  --> " << FilePath << ":" << Line+1 << ":" << Column+1 << std::endl;
        std::cerr << "    |" << std::endl;

        const auto code = sourceCode.empty() ? err.GetRange().FilePath : sourceCode;

        std::vector<std::string> lines;
        std::stringstream ss(code);
        std::string line;

        while (std::getline(ss, line))
            lines.push_back(line);

        if (Line > 0 && Line <= lines.size())
        {
            const std::string lineNumStr = std::to_string(Line);
            std::cerr << " " << lineNumStr << " | " << lines[Line] << std::endl;

            std::cerr << "    | ";
            for (int i = 0; i < Column+1; i++)
            {
                if (lines[Line][i] == '\t')
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
