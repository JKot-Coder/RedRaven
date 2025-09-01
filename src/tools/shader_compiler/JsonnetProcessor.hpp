#pragma once

#include "common/Result.hpp"

#include "nlohmann\json.hpp"

namespace RR
{
    class JsonnetProcessor
    {
    public:
        JsonnetProcessor();
        ~JsonnetProcessor();

        Common::RResult evaluateFile(const std::string& file, nlohmann::json& outputJson);
    };
}