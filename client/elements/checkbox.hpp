#pragma once
#include "item.hpp"
#include <string>
#include <imgui.h>

template<typename clickfunction>
class checkbox : public item {
    private:
    std::string text;
    bool enabled;
    clickfunction on_click;
    public:
    // rule of 5
    checkbox (std::string text, clickfunction click, bool enabled = false) {
        this->text = text;
        this->enabled = enabled;
    }
    checkbox (const checkbox& c) {
        this->text = c.text;
        this->enabled = c.enabled;
    }
    checkbox& operator=(const checkbox& c) {
        this->text = c.text;
        this->enabled = c.enabled;
        return *this;
    }
    checkbox (checkbox&& c) {
        this->text = std::move(c.text);
        this->enabled = std::move(c.enabled);
    }
    checkbox& operator=(checkbox&& c) {
        this->text = std::move(c.text);
        this->enabled = std::move(c.enabled);
        return *this;
    }
    void render () override {
        if (ImGui::Checkbox("Enable Feature", &enabled)) {
            clickfunction();
        }
    }
    bool getEnabled () {
        return enabled;
    }
};