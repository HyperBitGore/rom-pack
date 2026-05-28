#include "file.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

File::File () {
    this->file_length = 0;
}

std::vector<std::string> File::splitString (std::string str, std::string delimiter) {
    std::vector<std::string> out;
    size_t i = 0;
    size_t delim_len = delimiter.size();
    std::string current;
    while (i < str.size()) {
        if (std::isspace(str[i]) && current.size() > 0) {
            out.push_back(current);
            current.clear();
            i++;
        }
        else if (str.substr(i, delim_len) == delimiter) {
            out.push_back(current);
            current.clear();
            i += delim_len;
        } else {
            if (!std::isspace(str[i])) {
                current.push_back(str[i]);
            }
            i++;
        }
    }
    out.push_back(current);
    return out;
}

File::File (std::string file_path) {
    this->file_path = file_path;
    std::vector<std::string> split = splitString(file_path, "/");
    this->file_name = split[split.size() - 1];
    try {
        this->file_length = std::filesystem::file_size(file_path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "File size error: " << e.what() << "\n";
    }
}
// copy
File::File (const File& file) {
    this->file_length = file.file_length;
    this->file_name = file.file_name;
    this->file_path = file.file_path;
}
// get raw bytes of file
std::vector<uint8_t> File::getFileContents () {
    uint8_t* file_contents = new uint8_t[this->file_length];
    std::ifstream f(file_path, std::ios::binary);
    f.read((char*)file_contents, this->file_length);
    f.close();
    // make this not so slow
    std::vector<uint8_t> contents;
    for (unsigned long i = 0; i < this->file_length; i++) {
        contents.push_back(file_contents[i]);
    }
    delete[] file_contents;
    return contents;
}