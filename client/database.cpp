#include "database.hpp"
#include <fstream>
#include <stdexcept>

const std::string packId = "ROMPACK";

void game_db::load (game_db* db) {
    std::ifstream file;
    file.open("rom.pack", std::ios::binary);
    if (!file) {
        throw std::runtime_error("Can't open rom.pack file!");
    }
    std::array<char, 1024> read_array{};
    read_array.fill('\0');
    file.read(read_array.data(), sizeof(packId));
    std::string str(read_array.data());

    file.close();
}
void game_db::save (game_db* db) {
    std::ofstream file;
    file.open("rom.pack", std::ios::binary);
    if (!file) {
        throw std::runtime_error("Can't open rom.pack file!");
    }
    file.write(packId.c_str(), packId.size());
    auto games = db->get_games();
    file.write((char*)(games.size()), sizeof(games.size()));
    for (auto& i : games) {
        std::string path = i.folder_path.string();
        std::string executable = i.executable.string();
        size_t s = path.size();
        file.write((char*)s, sizeof(s));
        file.write(path.c_str(), path.size());
        s = executable.size();
        file.write((char*)s, sizeof(s));
        file.write(executable.c_str(), executable.size());
    }
    file.close();
}