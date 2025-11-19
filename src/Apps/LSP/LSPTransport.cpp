#include "LSPTransport.h"

namespace AlengLSP
{
    bool LSPTransport::ReadMessage(json &output)
    {
        std::string line;
        int contentLength = 0;

        while (std::getline(std::cin, line))
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) break;
            if (line.starts_with("Content-Length: "))
            {
                contentLength = std::stoi(line.substr(16));
            }
        }

        if (contentLength == 0) return false;

        std::vector<char> buffer(contentLength);
        std::cin.read(buffer.data(), contentLength);

        if (std::cin.gcount() != contentLength) return false;

        try
        {
            output = json::parse(buffer.begin(), buffer.end());
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[LSP Error] Failed to parser JSON: " << e.what() << std::endl;
            return false;
        }
    }

    void LSPTransport::SendMessage(const json& message)
    {
        const std::string content = message.dump();
        std::cout << "Content-Length: " << content.length() << "\r\n\r\n" << content << std::flush;
    }

}