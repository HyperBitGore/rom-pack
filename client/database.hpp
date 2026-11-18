#pragma once
#include <filesystem>
#include <vector>
// TODO
//  - categories

class game {
    private:
    public:
    std::filesystem::path folder_path;
    std::filesystem::path executable;
    // rule of 5
    game (std::filesystem::path folder_path, std::filesystem::path executable) {
        this->folder_path = folder_path;
        this->executable = executable;
    }
    game (const game& g) {
        this->folder_path = g.folder_path;
        this->executable = g.executable;
    }
    game& operator=(const game& g) {
        this->folder_path = g.folder_path;
        this->executable = g.executable;
        return *this;
    }
    game (game&& g) {
        this->folder_path = std::move(g.folder_path);
        this->executable = std::move(g.executable);
    }
    game& operator=(game&& g) {
        this->folder_path = std::move(g.folder_path);
        this->executable = std::move(g.executable);
        return *this;
    }
};

// singleton
class game_db {
    private:
    game_db () {};
    game_db (const game_db& c) = delete;
    game_db& operator=(const game_db& c) = delete;
    game_db (game_db&& c) = delete;
    game_db& operator=(game_db&& c) = delete;

    std::vector<game> games;
    public:
    static game_db& instance () {
        static game_db cur_p;
        return cur_p;
    }
    void load (game_db* db);
    void save (game_db* db);
    std::vector<game>& get_games () {
        return games;
    }
};