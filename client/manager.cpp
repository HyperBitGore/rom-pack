#include "manager.hpp"
#include <cstdlib>
#include <thread>
#include "imgui.h"

Manager::Manager () {

}
// copy
Manager::Manager (const Manager& m) {

}
// renders the ImGui
void Manager::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
    ImGui::Begin("Library");
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
    this->lib_mut.lock();
    
    this->lib_mut.unlock();
}

// updates the library, spawns a thread to update
void Manager::updateLibrary (SSL_CTX* ctx, std::string ip, uint32_t port) {
    std::thread updateThread(&Manager::getLibrary, this, ctx, ip, port);
    updateThread.detach();
}