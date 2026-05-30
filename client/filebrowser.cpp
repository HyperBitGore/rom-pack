#include "filebrowser.hpp"
#include "imgui.h"
#include <algorithm>
#include <filesystem>
#include <stdexcept>

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
        this->mode = RenderMode::UploadProgress;
    }
    ImGui::BeginTable(this->current_path.string().c_str(), 2);

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
    }

    ImGui::EndTable();
    ImGui::End();
}
void FileBrowser::renderUploadProgress() {
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(400, 400));
    ImGui::Begin("File Upload",  nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Progress: ");
    
    if (ImGui::Button("Cancel")) {
        this->mode = RenderMode::Select;
        this->display = false;
    }
    ImGui::End();
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
    display = !display;
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