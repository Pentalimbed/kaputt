#include "menu.h"

#include <imgui.h>

namespace kaputt
{
void drawCatMenu()
{
    if (ImGui::Begin("Kaputt Config Menu"))
    {
        ImGui::SetWindowSize({400, 600}, ImGuiCond_FirstUseEver);

        if (ImGui::BeginTabBar("##"))
        {
            if (ImGui::BeginTabItem("General")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Triggers")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Rules")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Animations")) { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
} // namespace kaputt