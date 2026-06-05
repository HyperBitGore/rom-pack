#pragma once
#include "file.hpp"
#include "../shared/socket_enums.hpp"
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
        std::vector<Category> library;
        std::string library_path;
    public:
        FileManager ();
        // copy
        FileManager (const FileManager& fm);
        // adds a file to manage
        void addFile (std::string file_path, std::string category, std::string folder);
        // adds a library file (structure which links to other files)
        void addFileLibrary ();
        // update the file library
        void updateFileLibrary ();
        // serializes the library so we can send to client
        std::vector<uint8_t> serialize ();
        // adds an uploading file, returns the id
        uint32_t addIncomingFile (std::string file_name, std::string folder, uint64_t file_size);
        // update incoming file
        void updateIncomingFile (uint32_t id, std::vector<uint8_t> block);
};