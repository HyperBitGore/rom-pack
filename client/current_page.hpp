#pragma once

#include "elements/elements.hpp"
#include <memory>

class current_page {
    public:
    static current_page& instance () {
        static current_page cur_p;
        return cur_p;
    } 
    void set_page (std::unique_ptr<elements> els) {
        previous = std::move(page_elements);
        next_page = std::move(els);
    }
    void back () {
        set_page(std::move(previous));
    }
    void render ();
    bool run = true;
    private:
    std::unique_ptr<elements> page_elements;
    std::unique_ptr<elements> next_page;
    std::unique_ptr<elements> previous;
    current_page () {};
    current_page (const current_page& c) = delete;
    current_page& operator=(const current_page& c) = delete;
    current_page (current_page&& c) = delete;
    current_page& operator=(current_page&& c) = delete;
};