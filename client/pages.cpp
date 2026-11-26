#include "pages.hpp"
#include "current_page.hpp"
#include "database.hpp"
#include "elements/button.hpp"
#include "elements/elements.hpp"
#include "elements/text.hpp"
#include "elements/checkbox_list.hpp"
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

class game_list final : public item {
    std::vector<std::pair<std::string, std::vector<game>>> sections;

public:
    game_list () {
        auto& db = game_db::instance();
        for (auto& category : db.getCategories()) {
            sections.emplace_back(category.getName(), category.getGames());
        }
        std::vector<game> uncategorized;
        for (const auto& entry : db.get_games()) {
            if (db.getCategories(entry).empty()) {
                uncategorized.push_back(entry);
            }
        }
        if (!uncategorized.empty()) {
            sections.emplace_back("Uncategorized", std::move(uncategorized));
        }
    }

    bool onclick (float, float) override {
        return false;
    }

    bool onkeydown (int) override {
        return false;
    }

    void render () override {
        if (ImGui::Button("Quit")) {
            current_page& current_page = current_page::instance();
            current_page.run = false;
        }
        for (const char* action : {"Add Game", "Add Category", "Settings", "Export"}) {
            const auto& style = ImGui::GetStyle();
            if (ImGui::GetItemRectMax().x + style.ItemSpacing.x +
                ImGui::CalcTextSize(action).x + 2 * style.FramePadding.x <=
                ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x) {
                ImGui::SameLine();
            }
            if (ImGui::Button(action)) {
                if (strcmp("Export", action) == 0) {
                    current_page& current_page = current_page::instance();
                    current_page.set_page(pages::constructExportGamesPage());
                } else {
                    ImGui::OpenPopup(action);
                }
            }
        }
        ImGui::Spacing();

        const int columns = std::clamp(
            static_cast<int>(ImGui::GetContentRegionAvail().x / 180.0f), 1, 8
        );
        int section_id = 0;
        for (const auto& [name, games] : sections) {
            ImGui::SeparatorText(name.c_str());
            ImGui::PushID(section_id++);
            if (!games.empty() && ImGui::BeginTable("Games", columns, ImGuiTableFlags_SizingStretchSame)) {
                int game_id = 0;
                for (const auto& entry : games) {
                    ImGui::TableNextColumn();
                    ImGui::PushID(game_id++);
                    const std::string label = entry.name + "###game";
                    if (ImGui::Button(label.c_str(), {-1.0f, 52.0f})) {
                        current_page::instance().set_page(pages::constructGameDetailsPage(entry));
                    }
                    if (ImGui::CalcTextSize(label.c_str(), nullptr, true).x +
                        2 * ImGui::GetStyle().FramePadding.x > ImGui::GetItemRectSize().x &&
                        ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                        ImGui::SetTooltip("%s", entry.name.c_str());
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::PopID();
        }
    }
};

std::unique_ptr<elements> pages::constructGameListPage () {
    auto page = std::make_unique<elements>();
    page->addItem(std::make_unique<game_list>());
    page->addItem(std::make_unique<game_add_popup>());
    page->addItem(std::make_unique<category_add_popup>());
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
        "Play", 0.0f, 0.0f, 100.0f, 40.0f, play_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Edit", 0.0f, 0.0f, 100.0f, 40.0f, edit_click, no_keydown
    ));
    page->addItem(std::make_unique<game_edit_popup>(game));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Delete", 0.0f, 0.0f, 100.0f, 40.0f, delete_click, no_keydown
    ));
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    return page;
}

std::unique_ptr<elements> pages::constructExportGamesPage () {
    auto page = std::make_unique<elements>();
    game_db& instance = game_db::instance();
    std::vector<game> games = instance.get_games();
    std::vector<std::string> names;
    for (auto& i : games) {
        names.push_back(i.name);
    }
    page->addItem(std::make_unique<checkboxlist>(0, 200, names), "list");
    const click_handler export_click = [p = page.get(), games](float, float) {
        checkboxlist* ptr = reinterpret_cast<checkboxlist*>(p->getItem("list"));
        auto checked = ptr->getChecked();
        std::vector<game> out;
        for (size_t i = 0; i < checked.size(); i++) {
            if (checked[i]) {
                out.push_back(games[i]);
            }
        }
        game_db::exportGameList(games);
        current_page& current_page = current_page::instance();
        current_page.set_page(constructGameListPage());
        return true;
    };
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Export", 0.0f, 0.0f, 100.0f, 40.0f, export_click, no_keydown
    ));
    const click_handler back_click = [](float, float) {
        current_page& current_page = current_page::instance();
        current_page.set_page(constructGameListPage());
        return true;
    };
    page->addItem(std::make_unique<button<click_handler, keydown_handler>>(
        "Back", 0.0f, 0.0f, 100.0f, 40.0f, back_click, no_keydown
    ));
    return page;
}