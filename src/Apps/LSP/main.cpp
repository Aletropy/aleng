#include "Analyzer.h"
#include "LSPTransport.h"
#include "Core/Parser.h"
#include "Core/Error.h"

using namespace AlengLSP;

static Analyzer g_Analyzer;

void ValidateTextDocument(LSPTransport &transport, const std::string &fileUri, const std::string &code)
{
    json diagnostics = json::array();

    try
    {
        Aleng::Parser parser(code, fileUri);
        const auto program = parser.ParseProgram();

        g_Analyzer.Analyze(*program);

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
                        {"textDocumentSync", 1},
                        {"completionProvider", {
                            {"resolveProvider", false},
                            {"triggerCharacters", { "." }}
                        }}
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
        }
        else if (method == "textDocument/completion") {
            json response;
            response["jsonrpc"] = "2.0";
            response["id"] = request["id"];

            int line = request["params"]["position"]["line"];
            line++;
            auto symbols = g_Analyzer.GetSymbolsAt(line);

            json items = json::array();

            for (const auto& symbol : symbols)
            {
                items.push_back({
                    {"label", symbol.Name},
                    {"kind", symbol.Kind == "Function" ? 3 : 6},
                    {"detail", "Defined on line " + std::to_string(symbol.lineDefined)},
                    {"insertText", symbol.Name}
                });
            }

            std::vector<std::string> keywords = {
                "If", "Else", "For", "While", "Fn", "Return", "Break", "Continue", "Import", "True", "False"
            };

            for (const auto& kw : keywords) {
                items.push_back({
                    {"label", kw},
                    {"kind", 14},
                    {"detail", "Aleng Keyword"},
                    {"insertText", kw}
                });
            }

            items.push_back({
                {"label", "Fn (Snippet)"},
                {"kind", 15}, // 15 = Snippet
                {"detail", "Function Definition"},
                {"insertText", "Fn ${1:name}(${2:args})\n\t$0\nEnd"},
                {"insertTextFormat", 2}, // 2 = Snippet
                {"filterText", "Fn"}
            });

            items.push_back({
                {"label", "For (Range)"},
                {"kind", 15},
                {"insertText", "For ${1:i} = ${2:0} .. ${3:10}\n\t$0\nEnd"},
                {"insertTextFormat", 2},
                {"filterText", "For"}
            });

            for (std::vector<std::string> builtins = {"Print", "Len", "Append", "Pop", "IsNumber", "IsString"}; const auto& b : builtins) {
                items.push_back({
                    {"label", b},
                    {"kind", 3}, // 3 = Function
                    {"detail", "Built-in Function"},
                    {"insertText", b + "($0)"},
                    {"insertTextFormat", 2}
                });
            }

            response["result"] = items;
            transport.SendMessage(response);
        }
        else if (method == "exit")
        {
            break;
        }
    }

    return 0;
}
