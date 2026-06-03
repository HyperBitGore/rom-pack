#include <cstdint>
#include <iostream>
#include <openssl/err.h>
#include "../shared/socket.hpp"
#include "filebrowser.hpp"
#include "g_engine/util/logging.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "g_engine/g_engine_2d.hpp"
#include "../shared/socket_enums.hpp"
#include "manager.hpp"
#include <signal.h>
#include <utility>


gore::g_engine_2d eng("ROM-Pack", 1024, 768,0, gore::LogType::NONE, "rom-pack.log", 1024, 768);
std::string username = "local";

void render() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
// issue is the comps not maintaining width and height
void windowResize (uint32_t w, uint32_t h) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DeltaTime = 1.0f / 60.0f;
}

bool helloMsg (TLSSocket* sock) {
    std::vector<uint8_t> buffer = { std::to_underlying(SocketConnectType::HELLO) };
    // send user name / login flow here
    for (auto& i : username) {
        buffer.push_back(i);
    }
    sock->send(&buffer[0], buffer.size() * sizeof(char));
    buffer = sock->recv(true);
    if (buffer.size() > 0 && buffer[0] == std::to_underlying(SocketConnectType::HELLO)) {
        return true;
    }
    return false;
}

// https://github.com/ocornut/imgui

int main() {
    eng.setRenderFunction(render);
    eng.setWindowResize(windowResize);
    eng.setMaintainViewport(true);
    signal(SIGPIPE, SIG_IGN);
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!SSL_CTX_load_verify_locations(ctx, "cert.pem", nullptr)) {
        std::cerr << "Failed to load cert.pem\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    TLSSocket client("127.0.0.1", 9001, ctx);
    if (!client.connect(10)) {
        std::cout << "Error connecting to server\n";
        SSL_CTX_free(ctx);
        return 1;
    }
    // convert this to some login thing
    std::cout << "Connected to server\n";
    if (!helloMsg(&client)) {
        std::cout << "Error in hello msg to server\n";
        SSL_CTX_free(ctx);
        client.close();
        return 1;
    }
    client.close();
    Manager mng;
    mng.updateLibrary(ctx, "127.0.0.1", 9001);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1024.0f, 768.0f);
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330 core");
    FileBrowser fb;
    fb.setConnection(ctx, "127.0.0.1", 9001);
    while (true) {
        eng.updateInputState();

        float dt = (float)eng.getDelta();
        io.DeltaTime = dt > 0.0f ? dt : 1.0f / 60.0f;
        gore::vec2 mouse = eng.getMousePos();
        io.MousePos = ImVec2(mouse.x, mouse.y);
        io.MouseDown[0] = eng.getMouseLeftDown();
        io.MouseDown[1] = eng.getMouseRightDown();
        io.MouseDown[2] = eng.getMouseMiddleDown();
        for (size_t i = 0; i < 256; i++) {
            if (eng.getKeyDown(i)) {
                io.AddInputCharacter(i);
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 10));
        ImGui::SetNextWindowSize(ImVec2(400, 400));
        if (mng.getDisplay()) {
            mng.render();
        }
        else if (fb.getDisplay()) {
            fb.setLibrary(mng.retrieveLibrary());
            fb.render();
        } else {
            ImGui::Begin("File Manager", nullptr, ImGuiWindowFlags_NoCollapse);
            if (ImGui::Button("Upload File")) {
                std::cout << "upload clicked\n";
                fb.toggleDisplay();
            }
            if (ImGui::Button("Launch Game")) {
                std::cout << "Launch Game\n";
                mng.toggleDisplay();
                mng.updateLibrary(ctx, "127.0.0.1", 9001);
            }
            ImGui::End();
        }
        ImGui::Render();

        if (!eng.updateWindow()) break;
        if (eng.getKeyDown(g_Escape)) break;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    SSL_CTX_free(ctx);
    return 0;
}
