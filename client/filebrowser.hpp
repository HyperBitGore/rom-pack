#pragma once
#include <filesystem>
#include <stdexcept>
#include <vector>
#include "imgui.h"

class FileBrowser {
    private:
        enum class FileType { Folder, File, Image, Binary, Script };
        struct File {
            std::string file_name;
            FileType type;
        };     
        std::filesystem::path current_path;
        std::vector<File> current_files;
    public:
        FileBrowser () {
            current_path = std::filesystem::current_path();
            updateFileListing();
        }
        FileBrowser (std::string path) {
            current_path = std::filesystem::path(path);
            if (!std::filesystem::exists(current_path)) {
                throw std::runtime_error("Path inputted into FileBrowser constructor doesn't exist! " + path);
            }
            updateFileListing();
        }
        void updateFileListing ();
        void render();

};