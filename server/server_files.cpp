#include "server_files.hpp"
#include "file.hpp"
#include <fstream>
#include <iostream>

FileManager::FileManager () {
    this->library_path = "rom-pack.library";
    library.push_back({"unknown"});
}
// copy
FileManager::FileManager (const FileManager& fm) {
    this->files = fm.files;
    this->library_path = fm.library_path;
}
// adds a file to manage
void FileManager::addFile (std::string file_path) {
    File f(file_path);
    files.push_back(f);
}
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// adds a library file (structure which links to other files)
void FileManager::addFileLibrary () {
    std::ifstream f(library_path);
    if (!f) {
        std::cerr << "No library file located in set path!\n";
        return;
    }
    library.clear();

    int depth = 0;
    Category current_cat;
    GameFolder current_folder;
    LaunchEntry current_entry;

    std::string line;
    while (getline(f, line)) {
        std::string t = trim(line);
        if (t.empty()) continue;

        if (t == "}") {
            depth--;
            if (depth == 2)      { current_folder.entries.push_back(current_entry); current_entry = {}; }
            else if (depth == 1) { current_cat.folders.push_back(current_folder);   current_folder = {}; }
            else if (depth == 0) { library.push_back(current_cat);                  current_cat = {}; }
        } else if (t.back() == '{') {
            std::string name = trim(t.substr(0, t.size() - 1));
            if (depth == 0)      current_cat.name = name;
            else if (depth == 1) current_folder.name = name;
            else if (depth == 2) current_entry.name = name;
            depth++;
        } else {
            if (depth == 3) current_entry.command = t;
        }
    }
    f.close();
}

void FileManager::updateFileLibrary () {
    std::ofstream f(library_path);
    if (!f) {
        std::cerr << "Failed to open library file for writing!\n";
        return;
    }
    for (auto& cat : library) {
        f << cat.name << " {\n";
        for (auto& folder : cat.folders) {
            f << "    " << folder.name << " {\n";
            for (auto& entry : folder.entries) {
                f << "        " << entry.name << " {\n";
                f << "            " << entry.command << "\n";
                f << "        }\n";
            }
            f << "    }\n";
        }
        f << "}\n";
    }
    f.close();
}

std::vector<uint8_t> FileManager::serialize () {
    std::vector<uint8_t> buffer;
    
    return buffer;
}

uint32_t FileManager::addIncomingFile (std::string file_name, uint64_t file_size) {
    uint32_t id = (uint32_t)incoming.size();
    IncomingFile inf;
    inf.f = File(file_name);
    inf.id = id;
    inf.file_size = file_size;
    incoming.push_back(inf);
    return id;
}

void FileManager::updateIncomingFile (uint32_t id, std::vector<uint8_t> block) {
    for (size_t i = 0; i < incoming.size(); i++) {
        if (incoming[i].id == id) {
            incoming[i].f.addBytes(block);
            if (block.size() < 1019) {
                files.push_back(incoming[i].f);
                incoming.erase(incoming.begin() + i);
            }
            return;
        }
    }
}