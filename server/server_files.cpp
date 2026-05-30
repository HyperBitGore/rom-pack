#include "server_files.hpp"
#include "file.hpp"

FileManager::FileManager () {

}
// copy
FileManager::FileManager (const FileManager& fm) {
    this->files = fm.files;
}
// adds a file to manage
void FileManager::addFile (std::string file_path) {
    File f(file_path);
    files.push_back(f);
}
// adds a library file (structure which links to other files)
void FileManager::addFileLibrary (std::string file_path) {
    // read the file into memory

    // parse the memory

    // add the files linked here into our file manager
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
    for (auto& i : incoming) {
        if (i.id == id) {
            i.f.addBytes(block);
        }
    }
}