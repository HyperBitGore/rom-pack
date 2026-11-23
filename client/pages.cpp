#include "pages.hpp"
#include "current_page.hpp"
#include "database.hpp"
#include "elements/button.hpp"
#include "elements/elements.hpp"
#include "elements/text.hpp"
#include <filesystem>
#include <imgui.h>
#include <portable-file-dialogs.h>
#include <memory>

class game_add_popup final : public item {
    char name[256]{};
    char launch_options[1024]{};

public:
    bool onclick (float, float) override {
        return false;
    }

    void render () override {
        if (!ImGui::BeginPopupModal("Add Game", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }

        ImGui::InputText("Name", name, sizeof(name));
        ImGui::InputText("Launch options", launch_options, sizeof(launch_options));
        if (ImGui::Button("Choose executable") && name[0] != '\0') {
            auto selection = pfd::open_file("Pick a game executable", ".").result();
            if (!selection.empty()) {
                std::filesystem::path file_path(selection[0]);
                game_db::instance().addGame(
                    file_path.parent_path(),
                    file_path.filename(),
                    name,
                    launch_options
                );
                ImGui::CloseCurrentPopup();
                current_page::instance().set_page(pages::constructGameListPage());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    bool onkeydown (int) override {
        return false;
    }
};

using click_handler = std::function<bool(float, float)>;
using keydown_handler =  std::function<bool(int)>;
const click_handler no_click = [](float, float) {
    return false;
};
const keydown_handler no_keydown = [](int) {
    return false;
};
std::unique_ptr<elements> pages::constructMainPage () {
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
    const click_handler back_click = [](float, float) {
        current_page& current_page = current_page::instance();
        current_page.set_page(constructMainPage());
        return true;
    };
    const click_handler add_click = [](float, float) {
        ImGui::OpenPopup("Add Game");
        return true;
    };
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Add Game", 0.0f, 0.0f, 100.0f, 40.0f, add_click, no_keydown
    ));
    page->addItem(std::make_unique<game_add_popup>());
    // loop games
    game_db& instance = game_db::instance();
    auto games = instance.get_games();
    for (auto& i : games) {
        const click_handler game_click = [game = i](float, float) {
            current_page& current_page = current_page::instance();
            current_page.set_page(constructGameDetailsPage(game));
            return true;
        };
        page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
            i.name, 0.0f, 0.0f, 0.0f, 40.0f, game_click, no_keydown
        ));
    }

    return page;
}

std::unique_ptr<elements> pages::constructGameDetailsPage (game game) {
    auto page = std::make_unique<elements>();
    const click_handler back_click = [](float, float) {
        current_page& current_page = current_page::instance();
        current_page.set_page(constructGameListPage());
        return true;
    };
    page->addItem(std::make_unique<text>(game.name, 0.0f, 0.0f, 100.0f, 40.0f));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    return page;
}