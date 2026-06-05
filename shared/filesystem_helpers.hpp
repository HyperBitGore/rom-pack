#pragma once
#include <filesystem>

class filehelper {
    public:
    filehelper() = delete;
    static uint64_t getDirectorySize (std::filesystem::path);
};