#pragma once
#include <vector>
#include <string>
enum class SocketConnectType { HELLO = 0, FILE_UPLOAD_BEGIN = 1, FILE_UPLOAD_BLOCK = 2, CATEGORIES = 3 };

struct LaunchEntry  { std::string name; std::string command; };
struct GameFolder   { std::string name; std::vector<LaunchEntry> entries; };
struct Category     { std::string name; std::vector<GameFolder> folders; };