#pragma once
#include <cstdint>
#include <vector>
#include <string>

enum class FileType { GameFolder, GameBinary, GameLaunchScript };

class File {
    private:
        std::string file_path;
        std::string file_name;
        unsigned long file_length;
        std::vector<File*> related_files;
        static std::vector<std::string> splitString (std::string string, std::string split);
    public:
        File ();
        File (std::string file_path);
        // copy
        File (const File& file);
        // get raw bytes of file
        std::vector<uint8_t> getFileContents ();
        // add bytes
        void addBytes (std::vector<uint8_t>& bytes);
};