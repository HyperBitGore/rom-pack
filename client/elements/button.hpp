#pragma once
#include "collision.hpp"
#include "item.hpp"
#include "imgui.h"
#include <string>
#include <utility>

template<typename clickfunction, typename keydownfunction>
class button : public item {
    private:
    float x;
    float y;
    float w;
    float h;
    std::string text;
    clickfunction on_click;
    keydownfunction on_keydown;
    public:
    button (
        std::string text,
        float x,
        float y,
        float w,
        float h,
        clickfunction on_click,
        keydownfunction on_keydown
    )
        : x{x},
          y{y},
          w{w},
          h{h},
          text{std::move(text)},
          on_click{on_click},
          on_keydown{on_keydown} {}
    bool onclick (float x, float y) override;
    void render () override;
    bool onkeydown (int key) override;
};

template<typename clickfunction, typename keydownfunction>
bool button<clickfunction, keydownfunction>::onclick (float x, float y) {
    if (!collides(x, y, 0.0f, 0.0f, this->x, this->y, this->w, this->h)) {
        return false;
    }
    return on_click(x, y);
}

template<typename clickfunction, typename keydownfunction>
void button<clickfunction, keydownfunction>::render () {
    if (ImGui::Button(this->text.c_str(), { this->w, this->h })) {
        on_click(this->x, this->y);
    }
}

template<typename clickfunction, typename keydownfunction>
bool button<clickfunction, keydownfunction>::onkeydown (int key) {
    return on_keydown(key);
}