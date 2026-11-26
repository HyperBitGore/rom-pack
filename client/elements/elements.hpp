#pragma once
#include "item.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class elements {
    private:
        std::vector<std::unique_ptr<item>> items;
        std::unordered_map<std::string, item*> item_map;
    public:
        void render ();
        void addItem (std::unique_ptr<item> item, std::string tag = "");
        item* getItem (std::string tag);
};