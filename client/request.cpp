#include "request.hpp"
#include "curl/easy.h"

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
    ProcessResponse& response_process,
    const std::map<std::string, std::string> header,
    REQUEST_METHOD method
) {
    if (!curl) {
        return -1;
    }

    ResponseContext context{&response_process, &url};
    switch (method) {
    case REQUEST_METHOD::PUT:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
            static_cast<long>(body.size()));
        break;
    case REQUEST_METHOD::POST:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // body
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
            static_cast<long>(body.size()));
        break;
    case REQUEST_METHOD::HEAD:
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
      break;
    case REQUEST_METHOD::GET:
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &request::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
    // header writing
    curl_slist *headers = nullptr;
    if (!header.empty()) {
        for (auto& i : header) {
            headers = curl_slist_append(headers, std::string(i.first + ": " + i.second).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    const CURLcode result = curl_easy_perform(curl);
    if (headers != nullptr) {
        curl_slist_free_all(headers);
    }
    if (result != CURLE_OK) {
        return -static_cast<int32_t>(result);
    }

    return 1;
}