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
    bool onclick (float x, float y) override {
        return false;
    }
    void render () override {
        ImGui::Text("%s", str.c_str());
    }
    bool onkeydown (int key) override {
        return false;
    }

};