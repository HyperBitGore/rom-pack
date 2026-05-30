#pragma once
#include "file.hpp"
#include <cstdint>
#include <vector>
#include <string>


class FileManager {
    private:
        std::vector<File> files;
        struct IncomingFile {
            File f;
            uint32_t id;
            uint64_t file_size;
        };
        std::vector<IncomingFile> incoming;
    public:
        FileManager ();
        // copy
        FileManager (const FileManager& fm);
        // adds a file to manage
        void addFile (std::string file_path);
        // adds a library file (structure which links to other files)
        void addFileLibrary (std::string file_path);
        // adds an uploading file, returns the id
        uint32_t addIncomingFile (std::string file_name, uint64_t file_size);
        // update incoming file
        void updateIncomingFile (uint32_t id, std::vector<uint8_t> block);
};