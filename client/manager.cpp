#include "manager.hpp"
#include <cstdlib>
#include <thread>
#include <utility>
#include "imgui.h"
#include "../shared/buffer.hpp"

Manager::Manager () {

}
// copy
Manager::Manager (const Manager& m) {
    this->display = m.display;
    this->library = m.library;
}
// renders the ImGui
void Manager::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
    ImGui::Begin("Library");
    if (ImGui::Button("Back")) {
        this->display = false;
    }
    if (lib_mut.try_lock()) {
        if (library.empty()) {
            ImGui::Text("No library loaded.");
        }
        for (auto& cat : library) {
            if (ImGui::TreeNode(cat.name.c_str())) {
                for (auto& folder : cat.folders) {
                    if (ImGui::TreeNode(folder.name.c_str())) {
                        for (auto& entry : folder.entries) {
                            ImGui::Text("%s", entry.name.c_str());
                            ImGui::SameLine();
                            std::string btn_id = "Launch##" + cat.name + folder.name + entry.name;
                            if (ImGui::Button(btn_id.c_str())) {
                                system(entry.command.c_str());
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        lib_mut.unlock();
    } else {
        ImGui::Text("Updating...");
    }
    ImGui::End();
}

void Manager::getLibrary (SSL_CTX* ctx, std::string ip, uint32_t port) {
    if (!ctx) return;
    TLSSocket sock(ip, port, ctx);
    if (!sock.connect(10)) return;
    Buffer b;
    b.addByte(std::to_underlying(SocketConnectType::CATEGORIES));
    b.addFourByte(0);
    sock.send(&b.getData()[0], b.getData().size());
    // get the file
    b = sock.recv(true);
    this->lib_mut.lock();
    // skip the CATEGORIES type byte
    b.offset = 1;
    std::vector<Category> newLibrary;
    while (b.offset < b.getData().size()) {
        Category cat;
        cat.name = b.readString();
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
            cat.folders.push_back(folder);
        }
        newLibrary.push_back(cat);
    }
    this->library = newLibrary;
    this->lib_mut.unlock();
}

// updates the library, spawns a thread to update
void Manager::updateLibrary (SSL_CTX* ctx, std::string ip, uint32_t port) {
    std::thread updateThread(&Manager::getLibrary, this, ctx, ip, port);
    updateThread.detach();
}