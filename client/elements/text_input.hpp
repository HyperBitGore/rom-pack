#pragma once
#include "collision.hpp"
#include "item.hpp"
#include "imgui.h"
#include <array>
#include <cstring>
#include <string>
#include <utility>

template<typename clickfunction, typename keydownfunction>
class text_input : public item {
    private:
    float x;
    float y;
    float w;
    float h;
    std::string text;
    std::string* bound_text = nullptr;
    std::array<char, 256> input{};
    clickfunction on_click;
    keydownfunction on_keydown;

    public:
    text_input (
        std::string text,
        float x,
        float y,
        float w,
        float h,
        clickfunction on_click,
        keydownfunction on_keydown,
        std::string* bound_text = nullptr
    )
        : x{x},
          y{y},
          w{w},
          h{h},
          text{std::move(text)},
          bound_text{bound_text},
          on_click{on_click},
          on_keydown{on_keydown} {
        std::strncpy(input.data(), this->text.c_str(), input.size() - 1);
    }

    bool onclick (float x, float y) override {
        collides(
            x, y, 0.0f, 0.0f, this->x, this->y, this->w, this->h
        );
        return on_click(x, y);
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

    bool onkeydown (int key) override {
        return on_keydown(key);
    }
    std::string input_text() {
        return std::string(input);
    }
};