#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class game {
    private:
    public:
    std::filesystem::path folder_path;
    std::filesystem::path executable;
    std::string name;
    std::string launch_options;
    std::string program_to_run;
    // rule of 5
    game (
        std::filesystem::path folder_path,
        std::filesystem::path executable,
        std::string name,
        std::string launch_options = {},
        std::string program_to_run = {}
    ) {
        this->folder_path = folder_path;
        this->executable = executable;
        this->name = name;
        this->launch_options = launch_options;
        this->program_to_run = program_to_run;
    }
    game (const game& g) {
        this->folder_path = g.folder_path;
        this->executable = g.executable;
        this->name = g.name;
        this->launch_options = g.launch_options;
        this->program_to_run = g.program_to_run;
    }
    game& operator=(const game& g) {
        this->folder_path = g.folder_path;
        this->executable = g.executable;
        this->name = g.name;
        this->launch_options = g.launch_options;
        this->program_to_run = g.program_to_run;
        return *this;
    }
    game (game&& g) {
        this->folder_path = std::move(g.folder_path);
        this->executable = std::move(g.executable);
        this->name = std::move(g.name);
        this->launch_options = std::move(g.launch_options);
        this->program_to_run = std::move(g.program_to_run);
    }
    game& operator=(game&& g) {
        this->folder_path = std::move(g.folder_path);
        this->executable = std::move(g.executable);
        this->name = std::move(g.name);
        this->launch_options = std::move(g.launch_options);
        this->program_to_run = std::move(g.program_to_run);
        return *this;
    }
};

class category {
    private:
    std::unordered_map<std::string, game> games;
    std::string name;
    public:
    category (std::string name) {
        this->name = name;
    }
    ~category () = default;
    // rule of 5
    category (const category& c) {
        this->games = c.games;
        this->name = c.name;
    }
    category& operator=(const category& c) {
        this->games = c.games;
        this->name = c.name;
        return *this;
    }
    category (category&& c) {
        this->games = std::move(c.games);
        this->name = std::move(c.name);
    }
    category& operator=(category&& c) {
        this->games = std::move(c.games);
        this->name = std::move(c.name);
        return *this;
    }
    std::string getName () {
        return name;
    }
    void addGame (game game);
    void removeGame (std::string name);
    std::vector<game> getGames();
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
    std::vector<category> categories;
    void removeFromCategories (game game);
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
        std::string launch_options,
        std::string program_to_run
    );
    void editGame (std::string old_name, game updated_game);
    void removeGame (std::string name);
};