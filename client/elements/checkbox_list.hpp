#pragma once
#include "item.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <imgui.h>

class checkboxlist : public item {
    private:
    std::vector<std::string> items;
    std::vector<uint8_t> checked;
    float width;
    float height;
    public:
    checkboxlist (
        float width,
        float height,
        std::vector<std::string> items
    )
        : width{width},
          height{height},
          items{items}
          {
            for (auto& i : items) {
                checked.push_back(0);
            }
          }
    void render () override {
        if (ImGui::BeginChild("CheckboxList", ImVec2(width, height), ImGuiChildFlags_Borders)) {
            for (int i = 0; i < items.size(); i++) {
                ImGui::PushID(i); // Ensure a unique ID per row
                ImGui::Checkbox(items[i].c_str(), (bool*)&checked[i]);
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
    std::vector<uint8_t> getChecked() {
        return checked;
    }
};