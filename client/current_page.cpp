#include "current_page.hpp"

void current_page::render () {
    if (next_page) {
        page_elements = std::move(next_page);
    }
    page_elements->render();
    if (next_page) {
        page_elements = std::move(next_page);
    }
}