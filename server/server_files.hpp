#pragma once
#include "file.hpp"
#include <vector>
#include <string>


class FileManager {
    private:
        std::vector<File> files;
    public:
        FileManager ();
        // copy
        FileManager (const FileManager& fm);
        // adds a file to manage
        void addFile (std::string file_path);
        // adds a library file (structure which links to other files)
        void addFileLibrary (std::string file_path);
};