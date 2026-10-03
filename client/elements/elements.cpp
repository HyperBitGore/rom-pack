#include "elements.hpp"

void elements::render () {
    for (auto& i : items) {
        i->render();
    }
}
void elements::addItem (std::unique_ptr<item> item) {
    items.push_back(std::move(item));
}