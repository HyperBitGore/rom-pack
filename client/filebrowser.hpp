#pragma once
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <atomic>
#include "imgui.h"
#include "../shared/socket.hpp"
#include "../shared/socket_enums.hpp"

class FileBrowser {
    private:
        enum class FileType { Folder, File, Image, Binary, Script };
        enum class RenderMode { Select, UploadProgress, Category_Selection };
        struct File {
            std::string file_name;
            FileType type;
        };     
        std::filesystem::path current_path;
        std::vector<File> current_files;
        std::vector<Category> library;
        float x = 0;
        float y = 0;
        bool display = false;
        RenderMode mode = RenderMode::Select;
        std::string selected_file = "";
        SSL_CTX* ctx = nullptr;
        std::string server_ip;
        uint32_t server_port = 0;
        uint32_t upload_id = 0;
        uint16_t upload_window_size = 0;
        std::atomic<float> upload_progress;
        int selected_cat_idx = -1;
        int selected_folder_idx = -1;
        std::string filetypeToString (FileType ft);
        FileType getType (std::filesystem::path path);
        void renderSelect ();
        void renderSelectCategory ();
        void renderUploadProgress();
        bool startUpload();
        bool beginFileUpload (std::filesystem::path file);
        bool beginFolderUpload (std::filesystem::path folder);
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
        void setConnection(SSL_CTX* ctx, std::string ip, uint32_t port) {
            this->ctx = ctx;
            this->server_ip = ip;
            this->server_port = port;
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
        void setLibrary(std::vector<Category> categories) {
            this->library = categories;
        }

};