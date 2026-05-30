#pragma once
#include <filesystem>
#include <stdexcept>
#include <vector>
#include "imgui.h"

class FileBrowser {
    private:
        enum class FileType { Folder, File, Image, Binary, Script };
        enum class RenderMode { Select, UploadProgress };
        struct File {
            std::string file_name;
            FileType type;
        };     
        std::filesystem::path current_path;
        std::vector<File> current_files;
        float x = 0;
        float y = 0;
        bool display = false;
        RenderMode mode = RenderMode::Select;
        std::string selected_file = "";
        std::string filetypeToString (FileType ft);
        FileType getType (std::filesystem::path path);
        void renderSelect ();
        void renderUploadProgress();
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
        void toggleDisplay ();
        void render();
        std::string getSelectedFile () {
            if (selected_file.empty()) return "";
            return (current_path / selected_file).string();
        }
        bool getDisplay () {
            return display;
        }

};