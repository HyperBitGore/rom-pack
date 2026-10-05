#pragma once
#include <cstdint>
#include <curl/curl.h>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class REQUEST_METHOD { PUT, POST, GET, HEAD };

class request {
    public:
        using ProcessResponse = std::function<size_t(
            const char*,
            size_t,
            size_t,
            const std::string&
        )>;

        static int32_t sendRequest(
            const std::string& url,
            CURL* curl,
            const std::vector<uint8_t>& body,
            ProcessResponse& response_process,
            const std::map<std::string, std::string> header = {},
            REQUEST_METHOD method = REQUEST_METHOD::GET
        );

    private:
        struct ResponseContext {
            ProcessResponse* process;
            const std::string* url;
        };

        static size_t writeCallback(
            char* data,
            size_t size,
            size_t count,
            void* user_data
        );

};
