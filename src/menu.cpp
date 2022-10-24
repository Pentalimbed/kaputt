#include "menu.h"
#include "animation.h"

#include <imgui.h>

std::string joinTags(const StrSet& tags, bool sorted = true)
{
    std::string result = "";
    uint16_t    count  = 0;
    if (sorted)
    {
        std::vector<std::string_view> taglist(tags.begin(), tags.end());
        std::ranges::sort(taglist);
        for (auto it = taglist.begin(); it != taglist.end(); ++it, ++count)
        {
            result += *it;
            if (count != taglist.size() - 1)
                result += ' ';
        }
    }
    else
    {
        for (auto it = tags.begin(); it != tags.end(); ++it, ++count)
        {
            result += *it;
            if (count != tags.size() - 1)
                result += ' ';
        }
    }

    return result;
}

namespace kaputt
{
static std::mutex  status_msg_mutex;
static std::string status_msg = "STATUS BAR";
void               setStatusMessage(std::string_view msg)
{
    std::scoped_lock l(status_msg_mutex);
    status_msg = msg;
}

void drawCatMenu()
{
    ImGui::ShowDemoWindow();

    if (ImGui::Begin("Kaputt Config Menu"))
    {
        ImGui::SetWindowSize({400, 600}, ImGuiCond_FirstUseEver);

        ImGui::BeginChild("main", {0.f, -ImGui::GetFontSize() - 2.f});
        if (ImGui::BeginTabBar("##"))
        {
            if (ImGui::BeginTabItem("General")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Triggers")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Rules")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Animations")) { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        status_msg_mutex.lock();
        ImGui::TextColored({0.7f, 0.7f, 0.7f, 1.0f}, status_msg.c_str());
        status_msg_mutex.unlock();
    }
    ImGui::End();
}

void drawAnimationMenu()
{
}
} // namespace kaputt