#pragma once
#include "item.hpp"
#include <string>

class button : public item {
    private:
    float x;
    float y;
    float w;
    float h;
    std::string text;
    public:
    bool onclick (float x, float y) override;
    void render () override;
    bool onkeydown (int key) override;
};