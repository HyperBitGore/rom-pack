#include "pages.hpp"
#include "current_page.hpp"
#include "elements/button.hpp"
#include "elements/text_input.hpp"
#include "request.hpp"
#include <fstream>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

namespace {
bool saveToken(const std::string& token) {
    std::ofstream file("rom-pack.token", std::ios::trunc);
    if (!file) {
        return false;
    }
    file << token;
    return file.good();
}

struct connection_state {
    std::string address{"http://127.0.0.1:8080"};
    std::string status{"Not connected"};
};

class status_text : public item {
    private:
    std::string* text;

    public:
    explicit status_text (std::string* text) : text{text} {}

    bool onclick (float, float) override {
        return false;
    }

    void render () override {
        ImGui::TextUnformatted(text->c_str());
    }

    bool onkeydown (int) override {
        return false;
    }
};
struct login_state {
    std::string username;
    std::string password;
    std::string status;
};
}

std::unique_ptr<elements> pages::constructLocalSelectionPage () {
    using click_handler = bool (*)(float, float);
    using keydown_handler = bool (*)(int);
    const keydown_handler no_keydown = [](int) {
        return false;
    };
    const click_handler local_click = [](float, float) {
        current_page::instance().set_page(pages::constructMainPage());
        return true;
    };
    const click_handler server_click = [](float, float) {
         current_page::instance().set_page(pages::constructServerConnectPage());
        return true;
    };

    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "local", 0.0f, 0.0f, 100.0f, 30.0f, local_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "connect", 0.0f, 0.0f, 100.0f, 30.0f, server_click, no_keydown
    ));
    return page;
}

std::unique_ptr<elements> pages::constructMainPage () {
    using click_handler = bool (*)(float, float);
    using keydown_handler = bool (*)(int);
    const click_handler no_click = [](float, float) {
        return false;
    };
    const keydown_handler no_keydown = [](int) {
        return false;
    };

    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "File", 0.0f, 0.0f, 100.0f, 30.0f, no_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Settings", 0.0f, 0.0f, 100.0f, 30.0f, no_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Quit", 0.0f, 0.0f, 100.0f, 30.0f, no_click, no_keydown
    ));
    return page;
}
std::unique_ptr<elements> pages::constructServerConnectPage () {
    using click_handler = std::function<bool(float, float)>;
    using keydown_handler = bool (*)(int);
    const click_handler no_click = [](float, float) {
        return false;
    };
    const click_handler back_click = [](float, float) {
        current_page::instance().set_page(pages::constructLocalSelectionPage());
        return true;
    };
    const keydown_handler no_keydown = [](int) {
        return false;
    };
    auto state = std::make_shared<connection_state>();
    const click_handler connect_click = [state](float, float) {
        state->status = "Connecting...";
        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            state->status = "Connection failed: unable to initialize curl";
            return true;
        }

        request::ProcessResponse response = [](
            const char*,
            size_t size,
            size_t count,
            const std::string&
        ) {
            return size * count;
        };
        const int result = request::sendRequest(
            state->address + "/alive", curl, {}, response
        );
        long status_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        curl_easy_cleanup(curl);

        if (result == 1 && status_code >= 200 && status_code < 300) {
            state->status = "Connected";
            current_page::instance().set_page(pages::constructLoginPage(state->address));
        } else {
            state->status = "Connection failed";
        }
        return true;
    };
    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<text_input<click_handler, keydown_handler>>(
        state->address, 0.0f, 0.0f, 250.0f, 30.0f, no_click, no_keydown,
        &state->address
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Connect", 0.0f, 0.0f, 100.0f, 30.0f, connect_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 30.0f, back_click, no_keydown
    ));
    page->addItem(std::make_unique<status_text>(&state->status));
    return page;
}

std::unique_ptr<elements> pages::constructLoginPage (std::string address) {
    using click_handler = std::function<bool(float, float)>;
    using keydown_handler = bool (*)(int);
    const click_handler no_click = [](float, float) {
        return false;
    };
    const click_handler back_click = [](float, float) {
        current_page::instance().set_page(pages::constructServerConnectPage());
        return true;
    };
    const keydown_handler no_keydown = [](int) {
        return false;
    };
    auto state = std::make_shared<login_state>();
    const click_handler login_click = [state, address](float, float) {
        state->status = "Connecting...";
        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            state->status = "Connection failed: unable to initialize curl";
            return true;
        }

        std::string response_body;
        request::ProcessResponse response = [&response_body](
            const char* data,
            size_t size,
            size_t count,
            const std::string&
        ) {
            response_body.append(data, size * count);
            return size * count;
        };
        const nlohmann::json credentials{
            {"username", state->username},
            {"password", state->password}
        };
        const std::string body = credentials.dump();
        const std::vector<uint8_t> body_bytes(body.begin(), body.end());
        const int result = request::sendRequest(
            address + "/login", curl, body_bytes, response,
            { {"Content-Type", "application/json"} }, REQUEST_METHOD::PUT
        );
        long status_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        curl_easy_cleanup(curl);

        if (result == 1 && status_code >= 200 && status_code < 300) {
            try {
                const auto response_json = nlohmann::json::parse(response_body);
                if (!saveToken(response_json.at("token").get<std::string>())) {
                    state->status = "Login failed: unable to store token";
                    return false;
                }
            } catch (const nlohmann::json::exception&) {
                state->status = "Login failed: invalid server response";
                return false;
            }
            state->status = "Logged In";
            current_page::instance().set_page(pages::constructMainPage());
            return true;
        } else {
            state->status = "Login failed";
        }
        return false;
    };
    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<text_input<click_handler, keydown_handler>>(
        state->username, 0.0f, 0.0f, 250.0f, 30.0f, no_click, no_keydown,
        &state->username
    ));
    page->addItem(std::make_unique<text_input<click_handler, keydown_handler>>(
        state->password, 0.0f, 0.0f, 250.0f, 30.0f, no_click, no_keydown,
        &state->password
    ));
    page->addItem(std::make_unique<status_text>(&state->status));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Login", 0.0f, 0.0f, 100.0f, 30.0f, login_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 30.0f, back_click, no_keydown
    ));
    return page;
}