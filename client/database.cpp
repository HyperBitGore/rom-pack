#include "database.hpp"
#include <fstream>
#include <stdexcept>

const std::string packId = "ROMPACK";
constexpr size_t named_pack_marker = static_cast<size_t>(-1);
constexpr size_t options_pack_marker = named_pack_marker - 1;

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
    size_t marker = 0;
    file.read(reinterpret_cast<char*>(&marker), sizeof(marker));
    if (!file) {
        throw std::runtime_error("Invalid rom.pack file!");
    }
    const bool has_names = marker == named_pack_marker || marker == options_pack_marker;
    const bool has_options = marker == options_pack_marker;
    size_t game_count = marker;
    if (has_names) {
        file.read(reinterpret_cast<char*>(&game_count), sizeof(game_count));
        if (!file) {
            throw std::runtime_error("Invalid rom.pack file!");
        }
    }
    games.clear();
    games.reserve(game_count);
    for (size_t i = 0; i < game_count; i++) {
        std::string path = readLengthEncodedString();
        std::string executable = readLengthEncodedString();
        std::string name = has_names
            ? readLengthEncodedString()
            : std::filesystem::path(executable).filename().string();
        std::string launch_options = has_options ? readLengthEncodedString() : "";
        games.emplace_back(path, executable, name, launch_options);
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
    size_t marker = options_pack_marker;
    file.write(reinterpret_cast<const char*>(&marker), sizeof(marker));
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
    }
    file.close();
}

void game_db::addGame (
    std::filesystem::path folder_path,
    std::filesystem::path executable,
    std::string name,
    std::string launch_options
) {
    games.push_back({folder_path, executable, name, launch_options});
    save();
}