#include "filebrowser.hpp"
#include <stdexcept>

void FileBrowser::updateFileListing () {
    std::vector<File> temp_files; // in case we get an error don't update the file listings
    try {
        for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
            File f = { entry.path().filename(), FileType::File};
            temp_files.push_back(f);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error(("Error listing files: " + std::string(e.what())));
    }
    current_files = temp_files;
}
void FileBrowser::render() {
    
}