#include "request.hpp"

size_t request::writeCallback(
    char* data,
    size_t size,
    size_t count,
    void* user_data
) {
    auto* context = static_cast<ResponseContext*>(user_data);
    return (*context->process)(data, size, count, *context->url);
}

int32_t request::sendRequest(
    const std::string& url,
    CURL* curl,
    const std::vector<uint8_t>& body,
    ProcessResponse& response_process
) {
    if (!curl) {
        return -1;
    }

    ResponseContext context{&response_process, &url};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &request::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);

    const CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        return -static_cast<int32_t>(result);
    }

    return 1;
}