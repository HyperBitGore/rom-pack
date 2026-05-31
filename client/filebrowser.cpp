#include "filebrowser.hpp"
#include "imgui.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <fstream>
#include <thread>

static const std::string imageFileExtensions[] = {
    ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".tiff", ".tif",
    ".webp", ".ico", ".svg", ".avif", ".heic", ".heif", ".raw",
    ".psd", ".xcf", ".tga", ".dds",
};
static const std::string binaryFileExtensions[] = {
    ".exe", ".dll", ".so", ".dylib", ".bin", ".elf", ".out",
    ".o", ".a", ".lib", ".obj", ".class", ".pyc", ".pyo",
    ".wasm", ".rom", ".iso", ".img", ".dmg", ".deb", ".rpm",
    ".apk", ".ipa", ".jar", ".war", ".ear",
};

FileBrowser::FileType FileBrowser::getType (std::filesystem::path path) {
    if (std::filesystem::is_directory(path)) {
        return FileType::Folder;
    }
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    for (auto& e : imageFileExtensions) {
        if (e == ext) {
            return FileType::Image;
        }
    }
    for (auto& e : binaryFileExtensions) {
        if (e == ext) {
            return FileType::Binary;
        }
    }
    return FileType::File;
}

void FileBrowser::updateFileListing () {
    std::vector<File> temp_files; // in case we get an error don't update the file listings
    try {
        for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
            File f = { entry.path().filename(), getType(entry.path())};
            temp_files.push_back(f);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error(("Error listing files: " + std::string(e.what())));
    }
    current_files = temp_files;
}

void FileBrowser::renderSelect () {
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(400, 400));
    ImGui::Begin("File Browser",  nullptr, ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Upload")) {
        if (!selected_file.empty() && startUpload()) {
            this->mode = RenderMode::UploadProgress;
        }
    }
    ImGui::BeginTable(this->current_path.string().c_str(), 3);

    if (current_path != current_path.root_path()) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable("..", false, ImGuiSelectableFlags_SpanAllColumns)) {
            auto prev = current_path;
            current_path = current_path.parent_path();
            selected_file = "";
            try { updateFileListing(); } catch (...) { current_path = prev; }
        }
        ImGui::TableNextColumn();
        ImGui::Text("Folder");
    }

    for (size_t i = 0; i < current_files.size(); i++) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        std::string label = current_files[i].file_name + "##" + std::to_string(i);
        bool is_selected = (selected_file == current_files[i].file_name);
        ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;
        if (ImGui::Selectable(label.c_str(), is_selected, flags)) {
            if (current_files[i].type == FileType::Folder) {
                auto prev = current_path;
                current_path = current_path / current_files[i].file_name;
                selected_file = "";
                try { updateFileListing(); } catch (...) { current_path = prev; }
                break;
            } else {
                selected_file = current_files[i].file_name;
            }
        }
        ImGui::TableNextColumn();
        ImGui::Text("%s", filetypeToString(current_files[i].type).c_str());
        ImGui::TableNextColumn();
        if (current_files[i].type != FileType::Folder) {
            ImGui::Text("%lu", std::filesystem::file_size(current_path / current_files[i].file_name));
        }
    }

    ImGui::EndTable();
    ImGui::End();
}
void FileBrowser::renderUploadProgress() {
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(400, 400));
    ImGui::Begin("File Upload",  nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Uploading: %s", selected_file.c_str());
    ImGui::Text("Transfer ID: %u, Window: %u bytes", upload_id, (uint32_t)upload_window_size);
    float progress = upload_progress.load();
    ImGui::Text("%.1f%%", progress * 100.0f);
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
    
    
    if (ImGui::Button("Close")) {
        this->mode = RenderMode::Select;
        this->display = false;
    }
    ImGui::End();
}

void uploadFile (std::string file_path, std::string ip, uint32_t port, SSL_CTX* ctx, uint32_t upload_id, uint64_t file_size, std::atomic<float>* progress, uint16_t window_size) {
    std::ifstream f(file_path, std::ios::binary);
    std::streamsize chunk_size = window_size - 5; // 1 type byte + 4 id bytes
    std::vector<char> buffer(chunk_size);
    uint64_t bytes_sent = 0;
    while (f.read(buffer.data(), chunk_size) || f.gcount() > 0) {
        std::streamsize bytes_read = f.gcount();
        TLSSocket sock(ip, port, ctx);
        if (!sock.connect(10)) return;
        std::vector<uint8_t> send_buffer = { std::to_underlying(SocketConnectType::FILE_UPLOAD_BLOCK)};
        send_buffer.push_back(upload_id & 0xFF);
        send_buffer.push_back((upload_id >> 8) & 0xFF);
        send_buffer.push_back((upload_id >> 16) & 0xFF);
        send_buffer.push_back((upload_id >> 24) & 0xFF);
        for (std::streamsize i = 0; i < bytes_read; i++) {
            send_buffer.push_back(buffer[i]);
        }
        std::cout << "send buffer size: " << bytes_read << "\n";
        sock.send(&send_buffer[0], send_buffer.size());
        sock.close();
        bytes_sent += bytes_read;
        if (file_size > 0) progress->store((float)bytes_sent / (float)file_size);
    }

    f.close();
}

bool FileBrowser::startUpload() {
    if (!ctx || selected_file.empty()) return false;
    TLSSocket sock(server_ip, server_port, ctx);
    if (!sock.connect(10)) return false;
    
    std::vector<uint8_t> buffer = { std::to_underlying(SocketConnectType::FILE_UPLOAD_BEGIN) };
    uint64_t size = std::filesystem::file_size(current_path / selected_file);
    buffer.push_back(size & 0xFF);
    buffer.push_back((size >> 8) & 0xFF);
    buffer.push_back((size >> 16) & 0xFF);
    buffer.push_back((size >> 24) & 0xFF);
    buffer.push_back((size >> 32) & 0xFF);
    buffer.push_back((size >> 40) & 0xFF);
    buffer.push_back((size >> 48) & 0xFF);
    buffer.push_back((size >> 56) & 0xFF);
    for (uint8_t c : selected_file) {
        buffer.push_back(c);
    }

    if (!sock.send(&buffer[0], buffer.size())) return false;

    std::vector<uint8_t> response = sock.recv(true);
    // expect: [FILE_UPLOAD_BEGIN, id(4B LE), window_size(2B LE)]
    if (response.size() < 7 || response[0] != std::to_underlying(SocketConnectType::FILE_UPLOAD_BEGIN)) {
        return false;
    }
    upload_id = (uint32_t)response[1]
              | ((uint32_t)response[2] << 8)
              | ((uint32_t)response[3] << 16)
              | ((uint32_t)response[4] << 24);
    upload_window_size = (uint16_t)response[5] | ((uint16_t)response[6] << 8);
    upload_progress.store(0.0f);
    std::filesystem::path p = current_path / selected_file;
    std::thread uploadThread(uploadFile, p.string(), server_ip, server_port, ctx, upload_id, size, &upload_progress, upload_window_size);
    uploadThread.detach();
    return true;
}

void FileBrowser::render() {
    if (display) {
        switch (mode) {
            case RenderMode::Select:
                renderSelect();
            break;
            case RenderMode::UploadProgress:
                renderUploadProgress();
            break;
        }
    }
}

void FileBrowser::toggleDisplay () {
    display = true;
    this->x = ImGui::GetWindowWidth() / 2;
    this->y = ImGui::GetWindowHeight() / 2;
    updateFileListing();
}

std::string FileBrowser::filetypeToString (FileType ft) {
    switch (ft) {
    case FileType::Folder:
        return "Folder";
    case FileType::File:
        return "File";
    case FileType::Image:
        return "Image";
    case FileType::Binary:
        return "Binary";
    case FileType::Script:
        return "Script";
    }
}