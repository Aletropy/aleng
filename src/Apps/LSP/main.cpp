#include "LSPTransport.h"
#include "Core/Parser.h"
#include "Core/Error.h"

using namespace AlengLSP;

void ValidateTextDocument(LSPTransport &transport, const std::string &fileUri, const std::string &code)
{
    json diagnostics = json::array();

    try
    {
        Aleng::Parser parser(code, fileUri);
        parser.ParseProgram();

        for (const auto &err: parser.GetErrors())
        {
            const auto loc = err.GetLocation();
            int line = std::max(0, loc.Line - 1);
            int col = std::max(0, loc.Column - 1);

            json diag;
            diag["range"] = {
                {"start", {{"line", line}, {"character", col}}},
                {"end", {{"line", line}, {"character", col + 5}}}
            };
            diag["severity"] = 1; // Error
            diag["source"] = "Aleng Parser";
            diag["message"] = err.what();

            diagnostics.push_back(diag);
        }
    } catch (const std::exception &e)
    {
        json diag;
        diag["range"] = {{"start", {{"line", 0}, {"character", 0}}}, {"end", {{"line", 0}, {"character", 1}}}};
        diag["severity"] = 1;
        diag["message"] = std::string("Compiler Fatal Error: ") + e.what();
        diagnostics.push_back(diag);
    }

    json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = "textDocument/publishDiagnostics";
    notification["params"] = {
        {"uri", fileUri},
        {"diagnostics", diagnostics}
    };

    transport.SendMessage(notification);
}

int main()
{
    LSPTransport transport;
    json request;

    std::cerr << "[Aleng LSP] Server Started." << std::endl;

    while (transport.ReadMessage(request))
    {
        if (!request.contains("method")) continue;

        std::string method = request["method"];

        if (method == "initialize")
        {
            json response;
            response["jsonrpc"] = "2.0";
            response["id"] = request["id"];
            response["result"] = {
                {
                    "capabilities", {
                        {"textDocumentSync", 1}
                    }
                },
                {
                    "serverInfo", {
                        {"name", "Aleng Language Server"},
                        {"version", "0.1.0"}
                    }
                }
            };
            transport.SendMessage(response);
        } else if (method == "textDocument/didOpen")
        {
            std::cerr << "[Aleng LSP] File Opened." << std::endl;
            auto params = request["params"]["textDocument"];
            std::string uri = params["uri"];
            std::string text = params["text"];

            ValidateTextDocument(transport, uri, text);
        } else if (method == "textDocument/didChange")
        {
            auto params = request["params"];
            std::string uri = params["textDocument"]["uri"];
            std::string text = params["contentChanges"][0]["text"];

            ValidateTextDocument(transport, uri, text);
        } else if (method == "exit")
        {
            break;
        }
    }

    return 0;
}
