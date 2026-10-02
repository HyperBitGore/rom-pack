#include "pages.hpp"
#include "elements/button.hpp"

std::unique_ptr<elements> pages::constructLocalSelectionPage () {
    using click_handler = bool (*)(float, float);
    using keydown_handler = bool (*)(int);

    const click_handler no_click = [](float, float) {
        return false;
    };
    const keydown_handler no_keydown = [](int) {
        return false;
    };

    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "local", 0.0f, 0.0f, 100.0f, 30.0f, no_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "connect", 0.0f, 0.0f, 100.0f, 30.0f, no_click, no_keydown
    ));
    return page;
}