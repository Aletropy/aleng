#pragma once

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AlengLSP
{
    class LSPTransport
    {
    public:
        bool ReadMessage(json& output);
        void SendMessage(const json& message);
    };
}