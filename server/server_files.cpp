#include "server_files.hpp"
#include "file.hpp"
#include "../shared/buffer.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

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
void FileManager::addFile (std::string file_path, std::string category, std::string folder) {
    int cat_index = -1;
    int folder_index = -1;
    std::string name = std::filesystem::path(file_path).filename();
    for (int i = 0; i < library.size(); i++) {
        if (library[i].name == category) {
            cat_index = i;
            break;
        }
    }
    if (cat_index == -1) {
        for (int i = 0; i < library.size(); i++ ) {
            if (library[i].name == "unknown") {
                cat_index = i;
                break;
            }
        }
    }
    for (int i = 0; i < library[cat_index].folders.size(); i++) {
        if (library[cat_index].folders[i].name == folder) {
            folder_index = i;
            break;
        }
    }
    if (cat_index > -1) {
        if (folder_index > -1) {
            library[cat_index].folders[folder_index].entries.push_back({ name, "" });
        } else {
            library[cat_index].unknown.entries.push_back({name, ""});
        }
    }
    File f(file_path);
    files.push_back(f);
    updateFileLibrary();
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
        updateFileLibrary();
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
            else if (depth == 1) {
                if (current_folder.name == "__unknown__") {
                    current_folder.name = "";
                    current_cat.unknown = current_folder;
                } else {
                    current_cat.folders.push_back(current_folder);
                }
                current_folder = {};
            }
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
    auto writeFolder = [&](const std::string& indent, const GameFolder& folder) {
        f << indent << folder.name << " {\n";
        for (auto& entry : folder.entries) {
            f << indent << "    " << entry.name << " {\n";
            f << indent << "        " << entry.command << "\n";
            f << indent << "    }\n";
        }
        f << indent << "}\n";
    };
    for (auto& cat : library) {
        f << cat.name << " {\n";
        for (auto& folder : cat.folders) {
            writeFolder("    ", folder);
        }
        if (!cat.unknown.entries.empty()) {
            GameFolder unknown_out = cat.unknown;
            unknown_out.name = "__unknown__";
            writeFolder("    ", unknown_out);
        }
        f << "}\n";
    }
    f.close();
}

std::vector<uint8_t> FileManager::serialize () {
    Buffer buf = LibraryFunctions::serializeLibrary(library);
    return buf.getData();
}

uint32_t FileManager::addIncomingFile (std::string file_name, std::string category, std::string folder, uint64_t file_size) {
    std::filesystem::path dest_dir = std::filesystem::path(category) / folder;
    std::filesystem::path dest_file = dest_dir / file_name;
    if (std::filesystem::exists(dest_file)) {
        throw std::runtime_error("Uploading duplicate file, rejecting connection!");
    }
    std::error_code ec;
    std::filesystem::create_directories(dest_dir, ec);
    if (ec) {
        throw std::runtime_error("Failed to create directory: " + ec.message());
    }
    uint32_t id = (uint32_t)incoming.size();
    IncomingFile inf(file_name, folder, category, id, file_size);
    incoming.push_back(inf);
    return id;
}

void FileManager::updateIncomingFile (uint32_t id, std::vector<uint8_t> block) {
    for (size_t i = 0; i < incoming.size(); i++) {
        if (incoming[i].id == id) {
            incoming[i].f.addBytes(block);
            if (block.size() < 1019) {
                addFile(incoming[i].f.getFilePath(), incoming[i].category, incoming[i].folder);
                incoming.erase(incoming.begin() + i);
            }
            return;
        }
    }
}

void FileManager::addCategory (std::string category) {
    library.push_back({ category });
    updateFileLibrary();
}