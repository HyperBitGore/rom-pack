#include "current_page.hpp"
#include "imgui.h"

void current_page::render () {
    if (next_page) {
        page_elements = std::move(next_page);
    }

    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin(
        "ROM-Pack",
        nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse
    );
    page_elements->render();
    ImGui::End();

    if (next_page) {
        page_elements = std::move(next_page);
    }
}