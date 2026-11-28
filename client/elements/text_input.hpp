#pragma once
#include "item.hpp"
#include "imgui.h"
#include <array>
#include <cstring>
#include <string>
#include <utility>

class text_input : public item {
    private:
    float x;
    float y;
    float w;
    float h;
    std::string text;
    std::string* bound_text = nullptr;
    std::array<char, 256> input{};

    public:
    text_input (
        std::string text,
        float x,
        float y,
        float w,
        float h,
        std::string* bound_text = nullptr
    )
        : x{x},
          y{y},
          w{w},
          h{h},
          text{std::move(text)},
          bound_text{bound_text} {
        std::strncpy(input.data(), this->text.c_str(), input.size() - 1);
    }

    void render () override {
        ImGui::SetNextItemWidth(this->w);
        ImGui::PushID(this);
        const bool changed = ImGui::InputText(
            "##text_input", input.data(), input.size()
        );
        ImGui::PopID();
        if (changed) {
            text = input.data();
            if (bound_text != nullptr) {
                *bound_text = text;
            }
        }
    }

    std::string input_text() {
        return std::string(input);
    }
};