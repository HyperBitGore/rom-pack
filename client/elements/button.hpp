#pragma once
#include "item.hpp"
#include "imgui.h"
#include <string>
#include <utility>

template<typename clickfunction>
class button : public item {
    private:
    float w;
    float h;
    std::string text;
    clickfunction on_click;
    public:
    button (
        std::string text,
        float w,
        float h,
        clickfunction on_click
    )
        : w{w},
          h{h},
          text{std::move(text)},
          on_click{on_click} {}
    void render () override;
};

template<typename clickfunction>
void button<clickfunction>::render () {
    if (ImGui::Button(this->text.c_str(), { this->w, this->h })) {
        on_click();
    }
}