#include "Analyzer.h"
#include "LSPTransport.h"
#include "Core/Parser.h"
#include "Core/Error.h"

#include <iostream>
#include <map>
#include <algorithm>

using json = nlohmann::json;
using namespace AlengLSP;

static Analyzer g_Analyzer;
static std::map<std::string, std::string> g_DocumentStore;

constexpr int INTERNAL_ERROR = -32603;

/**
 * Converts an internal 1-based Aleng SourceRange to a 0-based LSP Range object.
 * Ensures no negative indices are emitted.
 */
json ToLSPRange(const Aleng::SourceRange& range) {
    return {
        {"start", {
            {"line", std::max(0, range.Start.Line - 1)},
            {"character", std::max(0, range.Start.Column - 1)}
        }},
        {"end", {
            {"line", std::max(0, range.End.Line - 1)},
            {"character", std::max(0, range.End.Column - 1)}
        }}
    };
}

/**
 * Converts a 0-based LSP Position object to 1-based integer coordinates.
 */
void FromLSPPosition(const json& position, int& outLine, int& outCol) {
    outLine = position.value("line", 0) + 1;
    outCol = position.value("character", 0) + 1;
}


/**
 * Pipeline: Parse -> Analyze -> Publish Diagnostics.
 * This function is called whenever a document is opened or changed.
 * It ensures the Analyzer state is up to date and reports syntax errors to the client.
 */
void ProcessDocument(LSPTransport& transport, const std::string& uri, const std::string& content) {
    g_DocumentStore[uri] = content;
    json diagnostics = json::array();

    try {
        Aleng::Parser parser(content, uri);

        if (const auto program = parser.ParseProgram()) {
            g_Analyzer.Analyze(*program, uri);
        }

        for (const auto& err : parser.GetErrors()) {
            json diag;
            diag["range"] = ToLSPRange(err.GetRange());
            diag["severity"] = 1; // 1 = Error
            diag["source"] = "Aleng Parser";
            diag["message"] = err.what();
            diagnostics.push_back(diag);
        }

        // TODO: In the future, we can pull semantic diagnostics (type mismatch)
        // from the Analyzer and append them here.

    } catch (const std::exception& e) {
        json diag;
        diag["range"] = {
            {"start", {{"line", 0}, {"character", 0}}},
            {"end", {{"line", 0}, {"character", 1}}}
        };
        diag["severity"] = 1;
        diag["message"] = std::string("Language Server Internal Error: ") + e.what();
        diagnostics.push_back(diag);
    }

    json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = "textDocument/publishDiagnostics";
    notification["params"] = {
        {"uri", uri},
        {"diagnostics", diagnostics}
    };
    transport.SendMessage(notification);
}

int main() {
    LSPTransport transport;
    json request;

    std::cerr << "[Aleng LSP] Server Started." << std::endl;

    while (transport.ReadMessage(request)) {
        if (!request.contains("method")) continue;

        const std::string method = request["method"];
        const auto id = request.contains("id") ? request["id"] : json(nullptr);

        try {
            if (method == "initialize") {
                json response;
                response["jsonrpc"] = "2.0";
                response["id"] = id;

                json legend = {
                    {"tokenTypes", {
                        "variable",   // 0
                        "function",   // 1
                        "parameter",  // 2
                        "property",   // 3
                        "class",      // 4
                        "string",     // 5
                        "number",     // 6
                        "keyword",    // 7
                        "operator"    // 8
                    }},
                    {"tokenModifiers", {"declaration", "static"}}
                };

                response["result"] = {
                    {"capabilities", {
                        {"textDocumentSync", 1}, // 1 = Full Sync
                        {"hoverProvider", true},
                        {"definitionProvider", true},
                        {"referencesProvider", true},
                        {"completionProvider", {
                            {"resolveProvider", false},
                            {"triggerCharacters", { "." }}
                        }},
                        {"semanticTokensProvider", {
                            {"legend", legend},
                            {"full", true}
                        }}
                    }},
                    {"serverInfo", {
                        {"name", "Aleng Language Server"},
                        {"version", "0.2.0"}
                    }}
                };
                transport.SendMessage(response);
            }
            else if (method == "textDocument/didOpen") {
                auto params = request["params"]["textDocument"];
                ProcessDocument(transport, params["uri"], params["text"]);
            }
            else if (method == "textDocument/didChange") {
                auto params = request["params"];
                ProcessDocument(transport, params["textDocument"]["uri"], params["contentChanges"][0]["text"]);
            }
            else if (method == "textDocument/hover") {
                int line, col;
                FromLSPPosition(request["params"]["position"], line, col);
                std::string uri = request["params"]["textDocument"]["uri"];

                std::string markdown = g_Analyzer.GetHoverInfo(uri, line, col);

                json response = {{"jsonrpc", "2.0"}, {"id", id}, {"result", nullptr}};
                if (!markdown.empty()) {
                    response["result"] = {
                        {"contents", {
                            {"kind", "markdown"},
                            {"value", markdown}
                        }}
                    };
                }
                transport.SendMessage(response);
            }
            else if (method == "textDocument/definition") {
                int line, col;
                FromLSPPosition(request["params"]["position"], line, col);
                std::string uri = request["params"]["textDocument"]["uri"];

                auto sym = g_Analyzer.FindSymbolAt(uri, line, col);
                json response = {{"jsonrpc", "2.0"}, {"id", id}, {"result", nullptr}};

                if (sym) {
                    response["result"] = {
                        {"uri", uri},
                        {"range", ToLSPRange(sym->definitionRange)}
                    };
                }
                transport.SendMessage(response);
            }
            else if (method == "textDocument/references") {
                int line, col;
                FromLSPPosition(request["params"]["position"], line, col);
                std::string uri = request["params"]["textDocument"]["uri"];

                auto refs = g_Analyzer.GetReferences(uri, line, col);
                json locations = json::array();

                for (const auto& range : refs) {
                    locations.push_back({
                        {"uri", uri},
                        {"range", ToLSPRange(range)}
                    });
                }

                json response = {{"jsonrpc", "2.0"}, {"id", id}, {"result", locations}};
                transport.SendMessage(response);
            }
            else if (method == "textDocument/semanticTokens/full") {
                std::string uri = request["params"]["textDocument"]["uri"];
                json tokens = g_Analyzer.GetSemanticTokens(uri);

                json response = {
                    {"jsonrpc", "2.0"},
                    {"id", id},
                    {"result", { {"data", tokens} }}
                };
                transport.SendMessage(response);
            }
            else if (method == "textDocument/completion") {
                int line, col;
                FromLSPPosition(request["params"]["position"], line, col);
                std::string uri = request["params"]["textDocument"]["uri"];

                auto symbols = g_Analyzer.GetCompletions(uri, line, col);
                json items = json::array();

                for (const auto& sym : symbols) {
                    int kind = 6; // 6 Variable
                    if (sym->category == Symbol::Category::Function) kind = 3;
                    else if (sym->category == Symbol::Category::Parameter) kind = 6;

                    json item = {
                        {"label", sym->name},
                        {"kind", kind},
                        {"detail", sym->type ? sym->type->ToString() : "Unknown"},
                        {"insertText", sym->name}
                    };

                    if (sym->category == Symbol::Category::Function) {
                        item["insertText"] = sym->name + "($0)";
                        item["insertTextFormat"] = 2; // Snippet
                    }

                    items.push_back(item);
                }

                json response = {{"jsonrpc", "2.0"}, {"id", id}, {"result", items}};
                transport.SendMessage(response);
            }
            else if (method == "shutdown") {
                transport.SendMessage({{"jsonrpc", "2.0"}, {"id", id}, {"result", nullptr}});
            }
            else if (method == "exit") {
                break;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[LSP Error] Exception processing method " << method << ": " << e.what() << std::endl;
            if (!id.is_null()) {
                json errResponse = {
                    {"jsonrpc", "2.0"},
                    {"id", id},
                    {"error", {
                        {"code", INTERNAL_ERROR},
                        {"message", e.what()}
                    }}
                };
                transport.SendMessage(errResponse);
            }
        }
    }

    return 0;
}