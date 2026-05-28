#include "server_files.hpp"

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