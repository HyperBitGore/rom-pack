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
    this->mode = m.mode;
}

void Manager::renderMain() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
    ImGui::Begin("Library");
    if (ImGui::Button("Back")) {
        this->display = false;
    }
    if (ImGui::Button("Add Category")) {
        this->mode = Mode::Add;
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

void Manager::addCategory (std::string category) {
    if (!ctx) return;
    TLSSocket sock(ip, port, ctx);
    if (!sock.connect(10)) return;
    Buffer b;
    b.addByte(std::to_underlying(SocketConnectType::ADD_CATEGORY));
    b.addString(category);
    sock.send(&b.getData()[0], b.getData().size());
}

void Manager::renderAdd () {
    ImGui::Begin("Add Category");
    static char buf[128] = ""; 
    ImGui::InputText("Category Name", buf, IM_ARRAYSIZE(buf));
    if (ImGui::Button("Back")) {
        this->mode = Mode::Main;
    }
    if (ImGui::Button("Add")) {
        this->mode = Mode::Main;
        std::thread updateThread(&Manager::addCategory, this, buf);
        updateThread.detach();
    }
    ImGui::End();
}
void Manager::renderFolder () {
    ImGui::Begin("Game Folder");

    ImGui::End();
}
void Manager::renderLaunchEntry () {
    ImGui::Begin("Game Launch");

    ImGui::End();
}
// renders the ImGui
void Manager::render() {
    switch (mode) {
    case Mode::Main:
        renderMain();
        break;
    case Mode::Add:
        renderAdd();
        break;
    case Mode::GameFolder:
        renderFolder();
        break;
    case Mode::LaunchEntry:
        renderLaunchEntry();
      break;
    }
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
    // skip the CATEGORIES type byte
    b.offset = 1;
    auto newLibrary = LibraryFunctions::deserializeLibrary(b);
    this->lib_mut.lock();
    this->library = newLibrary;
    this->lib_mut.unlock();
}

// updates the library, spawns a thread to update
void Manager::updateLibrary (SSL_CTX* ctx, std::string ip, uint32_t port) {
    this->ctx = ctx;
    this->ip = ip;
    this->port = port;
    std::thread updateThread(&Manager::getLibrary, this, ctx, ip, port);
    updateThread.detach();
}