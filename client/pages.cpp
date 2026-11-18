#include "pages.hpp"
#include "current_page.hpp"
#include "elements/button.hpp"
#include "elements/elements.hpp"
#include <memory>

std::unique_ptr<elements> pages::constructMainPage () {
    using click_handler = bool (*)(float, float);
    using keydown_handler = bool (*)(int);
    const click_handler no_click = [](float, float) {
        return false;
    };
    const click_handler quit_click = [](float, float) {
        current_page& current_page = current_page::instance();
        current_page.run = false;
        return true;
    };
    const click_handler game_click = [](float, float) {
        current_page& current_page = current_page::instance();
        current_page.set_page(constructGameListPage());
        return true;
    };
    const keydown_handler no_keydown = [](int) {
        return false;
    };

    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Games", 0.0f, 0.0f, 100.0f, 30.0f, game_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Settings", 0.0f, 0.0f, 100.0f, 30.0f, no_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Quit", 0.0f, 0.0f, 100.0f, 30.0f, quit_click, no_keydown
    ));
    return page;
}

std::unique_ptr<elements> pages::constructGameListPage () {
    auto page = std::make_unique<elements>();
    using click_handler = bool (*)(float, float);
    using keydown_handler = bool (*)(int);
    const click_handler no_click = [](float, float) {
        return false;
    };
    const click_handler back_click = [](float, float) {
        current_page& current_page = current_page::instance();
        current_page.back();
        return true;
    };
    const keydown_handler no_keydown = [](int) {
        return false;
    };
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    // loop games


    return page;
}