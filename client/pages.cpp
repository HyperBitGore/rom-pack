#include "pages.hpp"
#include "current_page.hpp"
#include "database.hpp"
#include "elements/button.hpp"
#include "elements/elements.hpp"
#include "elements/text.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <imgui.h>
#include <portable-file-dialogs.h>
#include <memory>

static void render_category_dropdown (std::vector<bool>& selected) {
    auto& categories = game_db::instance().getCategories();
    selected.resize(categories.size());
    const auto count = std::ranges::count(selected, true);
    const std::string preview = count == 0
        ? "Uncategorized"
        : std::to_string(count) + " selected";
    ImGui::BeginDisabled(categories.empty());
    if (ImGui::BeginCombo("Categories", preview.c_str())) {
        for (size_t i = 0; i < categories.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            bool checked = selected[i];
            if (ImGui::Checkbox(categories[i].getName().c_str(), &checked)) {
                selected[i] = checked;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}

class game_add_popup final : public item {
    char name[256]{};
    char launch_options[1024]{};
    char launch_executable[1024]{};
    std::vector<bool> selected_categories;
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
        ImGui::InputText("Launch Executable", launch_executable, sizeof(launch_executable));
        if (ImGui::IsWindowAppearing()) {
            selected_categories.clear();
        }
        render_category_dropdown(selected_categories);
        if (ImGui::Button("Choose executable") && name[0] != '\0') {
            auto selection = pfd::open_file("Pick a game executable", ".").result();
            if (!selection.empty()) {
                std::filesystem::path file_path(selection[0]);
                game_db::instance().addGame(
                    file_path.parent_path(),
                    file_path.filename(),
                    name,
                    launch_options,
                    launch_executable,
                    selected_categories
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

class game_edit_popup final : public item {
    game original;
    std::filesystem::path game_path;
    char name[256]{};
    char launch_options[1024]{};
    char launch_executable[1024]{};
    std::vector<bool> selected_categories;
public:
    explicit game_edit_popup (game original)
        : original{std::move(original)},
          game_path{this->original.folder_path / this->original.executable} {
        std::snprintf(name, sizeof(name), "%s", this->original.name.c_str());
        std::snprintf(
            launch_options,
            sizeof(launch_options),
            "%s",
            this->original.launch_options.c_str()
        );
        std::snprintf(
            launch_executable,
            sizeof(launch_executable),
            "%s",
            this->original.program_to_run.c_str()
        );
    }

    bool onclick (float, float) override {
        return false;
    }

    void render () override {
        if (!ImGui::BeginPopupModal("Edit Game", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }

        ImGui::InputText("Name", name, sizeof(name));
        ImGui::InputText("Launch options", launch_options, sizeof(launch_options));
        ImGui::InputText("Launch Executable", launch_executable, sizeof(launch_executable));
        if (ImGui::IsWindowAppearing()) {
            selected_categories.clear();
            for (auto& category : game_db::instance().getCategories()) {
                selected_categories.push_back(category.gameExists(original.name));
            }
        }
        render_category_dropdown(selected_categories);
        if (ImGui::Button("Choose executable")) {
            auto selection = pfd::open_file("Pick a game executable", ".").result();
            if (!selection.empty()) {
                game_path = selection[0];
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save") && name[0] != '\0') {
            game updated{
                game_path.parent_path(),
                game_path.filename(),
                name,
                launch_options,
                launch_executable
            };
            game_db::instance().editGame(original.name, updated, selected_categories);
            ImGui::CloseCurrentPopup();
            current_page::instance().set_page(pages::constructGameDetailsPage(updated));
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

class category_add_popup final : public item {
    char name[256]{};
public:
    bool onclick (float, float) override {
        return false;
    }

    void render () override {
        if (!ImGui::BeginPopupModal("Add Category", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }

        ImGui::InputText("Name", name, sizeof(name));
        if (ImGui::Button("Add") && name[0] != '\0') {
            game_db::instance().addCategory(name);
            name[0] = '\0';
            ImGui::CloseCurrentPopup();
            current_page::instance().set_page(pages::constructGameListPage());
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
    const click_handler add_category = [](float, float) {
        ImGui::OpenPopup("Add Category");
        return true;
    };
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Add Game", 0.0f, 0.0f, 100.0f, 40.0f, add_click, no_keydown
    ));
     page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Add Category", 0.0f, 0.0f, 100.0f, 40.0f, add_category, no_keydown
    ));
    page->addItem(std::make_unique<game_add_popup>());
    page->addItem(std::make_unique<category_add_popup>());
    game_db& instance = game_db::instance();
    size_t row = 0;
    const auto add_game = [&](const game& entry) {
        const click_handler game_click = [game = entry](float, float) {
            current_page& current_page = current_page::instance();
            current_page.set_page(constructGameDetailsPage(game));
            return true;
        };
        page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
            entry.name + "###game_row_" + std::to_string(row++),
            0.0f, 0.0f, 0.0f, 40.0f, game_click, no_keydown
        ));
    };
    for (auto& category : instance.getCategories()) {
        page->addItem(std::make_unique<text>(
            category.getName(), 0.0f, 0.0f, 100.0f, 40.0f
        ));
        for (const auto& entry : category.getGames()) {
            add_game(entry);
        }
    }

    bool has_uncategorized = false;
    for (const auto& entry : instance.get_games()) {
        if (!instance.getCategories(entry).empty()) {
            continue;
        }
        if (!has_uncategorized) {
            page->addItem(std::make_unique<text>(
                "Uncategorized", 0.0f, 0.0f, 100.0f, 40.0f
            ));
            has_uncategorized = true;
        }
        add_game(entry);
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
    const click_handler play_click = [game](float, float) {
        std::string combined = game.folder_path.string() + "/" + game.executable.string();
        if (!game.program_to_run.empty()) {
            combined = game.program_to_run + " \"" + combined + "\"" + " " + game.launch_options;
        } else if (!game.launch_options.empty()) {
            combined = combined + " " + game.launch_options;
        }
        std::system(combined.c_str());
        return true;
    };
    
    const click_handler delete_click = [game](float, float) {
        game_db& instance = game_db::instance();
        instance.removeGame(game.name);
        current_page& current_page = current_page::instance();
        current_page.set_page(constructGameListPage());
        return true;
    };
    const click_handler edit_click = [game](float, float) {
        ImGui::OpenPopup("Edit Game");
        return true;
    };
    page->addItem(std::make_unique<text>(game.name, 0.0f, 0.0f, 100.0f, 40.0f));
    page->addItem(std::make_unique<text>(game.program_to_run, 0.0f, 0.0f, 100.0f, 40.0f));
    page->addItem(std::make_unique<text>(game.folder_path / game.executable, 0.0f, 0.0f, 100.0f, 40.0f));
    page->addItem(std::make_unique<text>(game.launch_options, 0.0f, 0.0f, 100.0f, 40.0f));
    game_db& instance = game_db::instance();
    std::vector<std::string> cats = instance.getCategories(game);
    page->addItem(std::make_unique<text>("Categories", 0.0f, 0.0f, 100.0f, 40.0f));
    for (auto& i : cats) {
        page->addItem(std::make_unique<text>(i, 0.0f, 0.0f, 100.0f, 40.0f));
    }
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Edit", 0.0f, 0.0f, 100.0f, 40.0f, edit_click, no_keydown
    ));
    page->addItem(std::make_unique<game_edit_popup>(game));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Play", 0.0f, 0.0f, 100.0f, 40.0f, play_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Delete", 0.0f, 0.0f, 100.0f, 40.0f, delete_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    return page;
}