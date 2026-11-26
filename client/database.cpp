#include "database.hpp"
#include <fstream>
#include <stdexcept>

const std::string packId = "ROMPACK";

void game_db::load () {
    std::ifstream file;
    file.open("rom.pack", std::ios::binary);
    if (!file) {
        throw std::runtime_error("Can't open rom.pack file!");
    }
    std::string tag(packId.size(), '\0');
    file.read(tag.data(), tag.size());
    if (!file || tag != packId) {
        throw std::runtime_error("Not a rom.pack file!");
    }
    auto readLengthEncodedString = [&]() {
        size_t size = 0;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (!file) {
            throw std::runtime_error("Invalid rom.pack file!");
        }
        std::string value(size, '\0');
        file.read(value.data(), value.size());
        if (!file) {
            throw std::runtime_error("Invalid rom.pack file!");
        }
        return value;
    };
    size_t game_count = 0;
    file.read(reinterpret_cast<char*>(&game_count), sizeof(game_count));
    if (!file) {
        throw std::runtime_error("Invalid rom.pack file!");
    }
    games.clear();
    games.reserve(game_count);
    for (size_t i = 0; i < game_count; i++) {
        std::string path = readLengthEncodedString();
        std::string executable = readLengthEncodedString();
        std::string name = readLengthEncodedString();
        std::string launch_options = readLengthEncodedString();
        std::string program_to_run = readLengthEncodedString();
        games.emplace_back(path, executable, name, launch_options, program_to_run);
    }
    size_t category_count = 0;
    file.read(reinterpret_cast<char*>(&category_count), sizeof(category_count));
    if (!file) {
        throw std::runtime_error("Invalid rom.pack file!");
    }
    categories.clear();
    categories.reserve(category_count);
    for (size_t i = 0; i < category_count; i++) {
        category& current = categories.emplace_back(readLengthEncodedString());
        size_t category_game_count = 0;
        file.read(reinterpret_cast<char*>(&category_game_count), sizeof(category_game_count));
        if (!file) {
            throw std::runtime_error("Invalid rom.pack file!");
        }
        for (size_t j = 0; j < category_game_count; j++) {
            std::string game_name = readLengthEncodedString();
            bool found = false;
            for (const game& current_game : games) {
                if (current_game.name == game_name) {
                    current.addGame(current_game);
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Invalid rom.pack file!");
            }
        }
    }
    file.close();
}
void game_db::save () {
    std::ofstream file;
    file.open("rom.pack", std::ios::binary);
    if (!file) {
        throw std::runtime_error("Can't open rom.pack file!");
    }
    file.write(packId.c_str(), packId.size());
    size_t s = games.size();
    file.write(reinterpret_cast<const char*>(&s), sizeof(s));
    for (auto& i : games) {
        std::string path = i.folder_path.string();
        std::string executable = i.executable.string();
        s = path.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        file.write(path.c_str(), path.size());
        s = executable.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        file.write(executable.c_str(), executable.size());
        s = i.name.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        file.write(i.name.c_str(), i.name.size());
        s = i.launch_options.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        file.write(i.launch_options.c_str(), i.launch_options.size());
        s = i.program_to_run.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        file.write(i.program_to_run.c_str(), i.program_to_run.size());
    }
    s = categories.size();
    file.write(reinterpret_cast<const char*>(&s), sizeof(s));
    for (auto& i : categories) {
        std::vector<game> cat = i.getGames();
        std::string name = i.getName();
        s = name.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        file.write(name.c_str(), name.size());
        s = cat.size();
        file.write(reinterpret_cast<const char*>(&s), sizeof(s));
        for (auto& j : cat) {
            s = j.name.size();
            file.write(reinterpret_cast<const char*>(&s), sizeof(s));
            file.write(j.name.c_str(), j.name.size());
        }
    }
    file.close();
}

void game_db::addGame (
    std::filesystem::path folder_path,
    std::filesystem::path executable,
    std::string name,
    std::string launch_options,
    std::string program_to_run,
    const std::vector<bool>& selected
) {
    games.push_back({folder_path, executable, name, launch_options, program_to_run});
    assignCategories(games.back(), selected);
    save();
}

void game_db::addCategory (std::string name) {
    categories.emplace_back(std::move(name));
    save();
}

void game_db::removeCategory (std::string name) {
    for (size_t i = 0; i < categories.size(); i++) {
        if (categories[i].getName() == name) {
            categories.erase(categories.begin() + i);
            save();
            return;
        }
    }
}

void game_db::editGame (
    std::string old_name,
    game updated_game,
    const std::vector<bool>& selected
) {
    for (game& current : games) {
        if (current.name == old_name) {
            removeFromCategories(current);
            current = std::move(updated_game);
            assignCategories(current, selected);
            save();
            return;
        }
    }
}

void game_db::removeGame (std::string name) {
    for (size_t i = 0; i < games.size(); i++) {
        if (name == games[i].name) {
            removeFromCategories(games[i]);
            games.erase(games.begin() + i);
            save();
            return;
        }
    }
}

void game_db::removeFromCategories (game game) {
    for (auto& i : categories) {
        i.removeGame(game.name);
    }
}

void game_db::assignCategories (const game& game, const std::vector<bool>& selected) {
    for (size_t i = 0; i < categories.size() && i < selected.size(); ++i) {
        if (selected[i]) {
            categories[i].addGame(game);
        }
    }
}

std::vector<std::string> game_db::getCategories (game game) {
    std::vector<std::string> r;
    for (auto& i : categories) {
        if (i.gameExists(game.name)) {
            r.push_back(i.getName());
        }
    }
    return r;
}

std::vector<std::string> game_db::getCategoryNames () {
    std::vector<std::string> r;
    for (auto& i : categories) {
        r.push_back(i.getName());
    }
    return r;
}

void game_db::addGameToCategory (std::string category_name, game game) {
    for (auto& i : categories) {
        if (i.getName() == category_name) {
            i.addGame(std::move(game));
            save();
            return;
        }
    }
}

void game_db::removeGameFromCategory (std::string category_name, game game) {
    for (auto& i : categories) {
        if (i.getName() == category_name) {
            i.removeGame(game.name);
            save();
            return;
        }
    }
}

void game_db::exportGameList (std::vector<game> games) {
    
}

void category::addGame (game game) {
    games.insert({game.name, game});
}
void category::removeGame (std::string name) {
    games.erase(name);
}
std::vector<game> category::getGames() {
    std::vector<game> ret;
    for (auto& i : games) {
        ret.push_back(i.second);
    }
    return ret;
}
bool category::gameExists (std::string name) {
    auto it = games.find(name);
    return it != games.end();
}