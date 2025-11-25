#include "Analyzer.h"
#include "LSPTransport.h"
#include "Core/Parser.h"
#include "Core/Error.h"
#include <sstream>
#include <map>

using namespace AlengLSP;

static Analyzer g_Analyzer;
static std::map<std::string, std::string> g_DocumentStore;

void ValidateTextDocument(LSPTransport &transport, const std::string &fileUri, const std::string &code)
{
    g_DocumentStore[fileUri] = code;
    json diagnostics = json::array();

    try
    {
        Aleng::Parser parser(code, fileUri);
        const auto program = parser.ParseProgram();

        g_Analyzer.Analyze(*program);

        for (const auto &err: parser.GetErrors())
        {
            const auto loc = err.GetRange();
            int line = std::max(0, loc.Line - 1);
            int col = std::max(0, loc.Column - 1);

            json diag;
            diag["range"] = {
                {"start", {{"line", line}, {"character", col}}},
                {"end", {{"line", line}, {"character", col + 5}}}
            };
            diag["severity"] = 1;
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

std::string GetLineText(const std::string& fullText, int lineNum) {
    std::stringstream ss(fullText);
    std::string segment;
    int current = 0;
    while(std::getline(ss, segment)) {
        if (current == lineNum) return segment;
        current++;
    }
    return "";
}

std::string GetVariableBeforeDot(const std::string& lineText, int column) {
    if (column <= 1) return "";
    int end = column - 2;
    int start = end;

    while (start >= 0 && (isalnum(lineText[start]) || lineText[start] == '_')) {
        start--;
    }
    return lineText.substr(start + 1, end - start);
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

            json legend = {
                {"tokenTypes", {"variable", "function", "parameter", "string", "number", "keyword", "operator"}},
                {"tokenModifiers", {"declaration", "defaultLibrary"}}
            };

            response["result"] = {
                {
                    "capabilities", {
                        {"textDocumentSync", 1}, // Full Sync
                        {"completionProvider", {
                            {"resolveProvider", false},
                            {"triggerCharacters", { "." }}
                        }},
                        {"semanticTokensProvider", {
                            {"legend", legend},
                            {"full", true}
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
        }
        else if (method == "textDocument/didOpen")
        {
            auto params = request["params"]["textDocument"];
            ValidateTextDocument(transport, params["uri"], params["text"]);
        }
        else if (method == "textDocument/didChange")
        {
            auto params = request["params"];
            ValidateTextDocument(transport, params["textDocument"]["uri"], params["contentChanges"][0]["text"]);
        }
        else if (method == "textDocument/completion") {
            json response;
            response["jsonrpc"] = "2.0";
            response["id"] = request["id"];

            int line = request["params"]["position"]["line"];
            int character = request["params"]["position"]["character"];
            std::string uri = request["params"]["textDocument"]["uri"];

            json items = json::array();
            bool handled = false;

            bool isDotTrigger = false;
            if (request["params"].contains("context")) {
                if (auto ctx = request["params"]["context"]; ctx["triggerKind"] == 2 && ctx["triggerCharacter"] == ".") {
                    isDotTrigger = true;
                }
            }

            if (isDotTrigger && g_DocumentStore.contains(uri)) {
                std::string lineText = GetLineText(g_DocumentStore[uri], line);
                std::string varName = GetVariableBeforeDot(lineText, character);

                if (auto symbol = g_Analyzer.FindSymbol(varName, line + 1); symbol && symbol->Type.Name == "Map") {
                    for (const auto& key : symbol->Type.MapKeys) {
                        items.push_back({
                            {"label", key},
                            {"kind", 10}, // 10 = Property
                            {"detail", "Map Key"},
                            {"insertText", key}
                        });
                    }
                    handled = true;
                }
            }

            if (!handled) {
                for (auto symbols = g_Analyzer.GetSymbolsAt(line + 1); const auto& symbol : symbols)
                {
                    std::string detail = symbol.Kind;
                    if (symbol.Type.Name != "Unknown") {
                        detail += ": " + symbol.Type.Name;
                    }
                    if (symbol.Type.Name == "Function" && !symbol.Type.Signature.empty()) {
                        detail = symbol.Type.Signature;
                    }

                    json item = {
                        {"label", symbol.Name},
                        {"kind", symbol.Kind == "Function" ? 3 : 6}, // 3=Function, 6=Variable
                        {"detail", detail},
                        {"documentation", "Defined on line " + std::to_string(symbol.LineDefined)}
                    };

                    if (symbol.Kind == "Function") {
                        item["insertText"] = symbol.Name + "($0)";
                        item["insertTextFormat"] = 2;
                    } else {
                        item["insertText"] = symbol.Name;
                    }

                    items.push_back(item);
                }

                // Keywords
                std::vector<std::string> keywords = {
                    "If", "Else", "For", "While", "Fn", "Return", "Break", "Continue", "Import", "True", "False"
                };
                for (const auto& kw : keywords) {
                    items.push_back({
                        {"label", kw},
                        {"kind", 14}, // Keyword
                        {"detail", "Keyword"},
                        {"insertText", kw}
                    });
                }

                items.push_back({
                    {"label", "Fn (Snippet)"},
                    {"kind", 15},
                    {"detail", "Function Definition"},
                    {"insertText", "Fn ${1:name}(${2:args})\n\t$0\nEnd"},
                    {"insertTextFormat", 2},
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
                        {"kind", 3},
                        {"detail", "Built-in Function"},
                        {"insertText", b + "($0)"},
                        {"insertTextFormat", 2}
                    });
                }
            }

            response["result"] = items;
            transport.SendMessage(response);
        }
        else if (method == "textDocument/semanticTokens/full") {
            json response;
            response["jsonrpc"] = "2.0";
            response["id"] = request["id"];

            response["result"] = {
                {"data", g_Analyzer.GetSemanticTokens()}
            };

            transport.SendMessage(response);
        }
        else if (method == "exit")
        {
            break;
        }
    }

    return 0;
}