#include "socket_enums.hpp"

std::vector<Category> LibraryFunctions::deserializeLibrary (Buffer b) {
    std::vector<Category> newLibrary;
    size_t size = b.readFourByte();
    for (size_t i = 0; i < size; i++) {
        Category cat;
        cat.name = b.readString();
        if (cat.name.empty()) {
            continue;
        }
        uint32_t num_folders = b.readFourByte();
        for (uint32_t i = 0; i < num_folders; i++) {
            GameFolder folder;
            folder.name = b.readString();
            uint32_t num_entries = b.readFourByte();
            for (uint32_t j = 0; j < num_entries; j++) {
                LaunchEntry entry;
                entry.name = b.readString();
                entry.command = b.readString();
                folder.entries.push_back(entry);
            }
            if (folder.name == "__unknown__") {
                cat.unknown = folder;
            } else {
                cat.folders.push_back(folder);
            }
        }
        newLibrary.push_back(cat);
    }
    return newLibrary;
}
Buffer LibraryFunctions::serializeLibrary (std::vector<Category> lib) {
    Buffer buf;
    auto writeFolder = [&] (GameFolder* gf) {
        buf.addString(gf->name);
        buf.addFourByte(gf->entries.size());
        for (auto& game : gf->entries) {
                buf.addString(game.name);
                buf.addString(game.command);
        }
    };
    buf.addFourByte(lib.size());
    for (auto& cat : lib) {
        buf.addString(cat.name);
        buf.addFourByte(cat.folders.size());
        writeFolder(&cat.unknown);
        for (auto& folder : cat.folders) {
            writeFolder(&folder);
        }
    }
    return buf;
}