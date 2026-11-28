#pragma once
#include "item.hpp"
#include "imgui.h"
#include <string>

class text : public item {
    private:
    float x;
    float y;
    float w;
    float h;
    std::string str;
    public:
    text (
        std::string str,
        float x,
        float y,
        float w,
        float h
    )
        : x{x},
          y{y},
          w{w},
          h{h},
          str{std::move(str)} {}
    void render () override {
        ImGui::Text("%s", str.c_str());
    }

};