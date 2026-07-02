#pragma once
#include "buffer.hpp"
#include <vector>
#include <string>
enum class SocketConnectType { HELLO, FILE_UPLOAD_BEGIN, FOLDER_UPLOAD_BEGIN, FILE_UPLOAD_BLOCK, CATEGORIES, ADD_CATEGORY };

struct LaunchEntry  { std::string name; std::string command; };
struct GameFolder   { std::string name; std::vector<LaunchEntry> entries; };
struct Category     { std::string name; std::vector<GameFolder> folders; GameFolder unknown; };

class LibraryFunctions {
    private:

    public:
    LibraryFunctions() = delete;
    static std::vector<Category> deserializeLibrary (Buffer b);
    static Buffer serializeLibrary (std::vector<Category> lib);
};