#include "Error.h"

namespace Aleng
{
    void PrintFormattedError(const AlengError &err, const std::string &sourceCode, const std::string &filePath)
    {
        std::cerr << "Runtime Error: " << err.what() << std::endl;

        auto loc = err.GetLocation();
        std::cerr << "  --> " << filePath << ":" << loc.Line << ":" << loc.Column << std::endl;
        std::cerr << "    |" << std::endl;

        std::vector<std::string> lines;
        std::stringstream ss(sourceCode);
        std::string line;

        while (std::getline(ss, line))
            lines.push_back(line);

        if (loc.Line > 0 && loc.Line <= lines.size())
        {
            std::string lineNumStr = std::to_string(loc.Line);
            std::cerr << " " << lineNumStr << " | " << lines[loc.Line - 1] << std::endl;

            std::cerr << "    | ";
            for (int i = 0; i < loc.Column; i++)
            {
                if (lines[loc.Line - 1][i] == '\t')
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