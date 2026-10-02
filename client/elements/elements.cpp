#include "elements.hpp"

void elements::render () {
    for (auto& i : items) {
        i->render();
    }
}
void elements::onclick (float x, float y) {
    for (auto& i : items) {
        bool clicked = i->onclick(x, y);
        if (clicked) {
            return;
        }
    }
}
void elements::addItem (std::unique_ptr<item> item) {
    items.push_back(std::move(item));
}