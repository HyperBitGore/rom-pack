#include "filesystem_helpers.hpp"
#include <cstdint>
#include <stdexcept>


uint64_t filehelper::getDirectorySize (std::filesystem::path path) {
    if (!std::filesystem::is_directory(path)) {
        throw std::runtime_error(path.string() + std::string(" is not a directory!"));
    }
    uint64_t total_size = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (std::filesystem::is_regular_file(entry)) {
            total_size += std::filesystem::file_size(entry);
        }
    }
    return total_size;
}