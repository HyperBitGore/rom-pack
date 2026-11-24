#pragma once
#include <filesystem>
#include <string>
#include <vector>
// TODO
//  - categories

class game {
    private:
    public:
    std::filesystem::path folder_path;
    std::filesystem::path executable;
    std::string name;
    std::string launch_options;
    // rule of 5
    game (
        std::filesystem::path folder_path,
        std::filesystem::path executable,
        std::string name,
        std::string launch_options = {}
    ) {
        this->folder_path = folder_path;
        this->executable = executable;
        this->name = name;
        this->launch_options = launch_options;
    }
    game (const game& g) {
        this->folder_path = g.folder_path;
        this->executable = g.executable;
        this->name = g.name;
        this->launch_options = g.launch_options;
    }
    game& operator=(const game& g) {
        this->folder_path = g.folder_path;
        this->executable = g.executable;
        this->name = g.name;
        return *this;
    }
    game (game&& g) {
        this->folder_path = std::move(g.folder_path);
        this->executable = std::move(g.executable);
        this->name = std::move(g.name);
        this->launch_options = std::move(g.launch_options);
    }
    game& operator=(game&& g) {
        this->folder_path = std::move(g.folder_path);
        this->executable = std::move(g.executable);
        this->name = std::move(g.name);
        this->launch_options = std::move(g.launch_options);
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
    void load ();
    void save ();
    std::vector<game>& get_games () {
        return games;
    }
    void addGame (
        std::filesystem::path folder_path,
        std::filesystem::path executable,
        std::string name,
        std::string launch_options
    );
    void removeGame (std::string name);
};