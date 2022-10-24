#include "menu.h"
#include "animation.h"

#include <imgui.h>
#include "extern/imgui_stdlib.h"

namespace kaputt
{
static std::mutex  status_msg_mutex;
static std::string status_msg = "Kaputt Ver. " + SKSE::PluginDeclaration::GetSingleton()->GetVersion().string();
void               setStatusMessage(std::string_view msg)
{
    std::scoped_lock l(status_msg_mutex);
    status_msg = msg;
}

void drawAnimationMenu()
{
    static std::string filter_text = {};
    int                filter_mode = 0; // 0 None 1 ID 2 Tags
    // static std::string selected_edid = ""; // Selected anim entry in the list

    auto anim_manager = AnimEntryManager::getSingleton();

    // file operation
    if (ImGui::BeginTable("filetab", 2))
    {
        ImGui::TableNextColumn();
        if (ImGui::Button("Save Custom Tags", {-FLT_MIN, 0.f}))
            anim_manager->saveCustomFile(anim_manager->getDefaultCustomFilePath());

        ImGui::TableNextColumn();
        if (ImGui::Button("Reload Custom Tags", {-FLT_MIN, 0.f}))
            anim_manager->loadCustomFile(anim_manager->getDefaultCustomFilePath());

        ImGui::EndTable();
    }
    ImGui::Separator();

    // filters
    ImGui::InputText("Filter by", &filter_text);

    if (ImGui::BeginTable("filtertab", 3))
    {
        ImGui::TableNextColumn();
        ImGui::RadioButton("None", &filter_mode, 0);
        ImGui::TableNextColumn();
        ImGui::RadioButton("ID", &filter_mode, 1);
        ImGui::TableNextColumn();
        ImGui::RadioButton("Tag", &filter_mode, 2);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Seperate each tag with SPACE.");

        ImGui::EndTable();
    }

    // list of anims
    constexpr auto table_flags = ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("Animation Entries", 2, table_flags, {0.f, -FLT_MIN}))
    {
        ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthStretch, 0.4);
        ImGui::TableSetupColumn("Tags", 0, 0.6);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();

        for (auto& [edid, anim] : anim_manager->anim_dict)
        {
            ImGui::PushID(edid.c_str());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            if (ImGui::Selectable(edid.c_str(), false))
                ; // TODO
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Click to perform with the nearest NPC.");

            ImGui::TableNextColumn();
            auto tags_str = joinTags(anim.getTags());
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##", &tags_str, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (tags_str.empty())
                    anim.custom_tags.reset();
                else
                    anim.custom_tags = splitTags(tags_str);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Press Enter to apply tag editing.\n"
                                  "The tags will be sorted.\n"
                                  "Leave empty and press Enter to set to default.\n"
                                  "(Remember to save to the file afterwards.)");

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void drawCatMenu()
{
    ImGui::ShowDemoWindow();

    if (ImGui::Begin("Kaputt Config Menu", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::SetWindowSize({400, 600}, ImGuiCond_FirstUseEver);

        ImGui::BeginChild("main", {0.f, -ImGui::GetFontSize() - 2.f});
        if (ImGui::BeginTabBar("##"))
        {
            if (ImGui::BeginTabItem("General")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Triggers")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Rules")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Animations"))
            {
                drawAnimationMenu();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        status_msg_mutex.lock();
        ImGui::TextColored({0.7f, 0.7f, 0.7f, 1.0f}, status_msg.c_str());
        status_msg_mutex.unlock();
    }
    ImGui::End();
}
} // namespace kaputt