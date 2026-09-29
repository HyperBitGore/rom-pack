#pragma once
#include "item.hpp"
#include <memory>
#include <vector>

class elements {
    private:
        std::vector<std::unique_ptr<item>> items;
    public:
        void render ();
        void onclick (float x, float y);
        void addItem (std::unique_ptr<item> item);
};