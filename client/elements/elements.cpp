#include "elements.hpp"

void elements::render () {
    for (auto& i : items) {
        i->render();
    }
}
void elements::addItem (std::unique_ptr<item> item, std::string tag) {
    if (!tag.empty()) {
        item_map.insert({tag, item.get()});
    }
    items.push_back(std::move(item));
}
item* elements::getItem (std::string tag) {
    auto it = item_map.find(tag);
    if (it != item_map.end()) {
        return it->second;
    }
    return nullptr;
}