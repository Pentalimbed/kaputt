#include "menu.h"

#include "re.h"
#include "utils.h"
#include "animation.h"

#include <imgui.h>
#include <imgui_stdlib.h>

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
    static std::string         filter_text = {};
    static ImGuiTableSortSpecs sort_specs  = {};
    static int                 filter_mode = 0; // 0 None 1 ID 2 Tags

    auto anim_manager = AnimManager::getSingleton();

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
            ImGui::SetTooltip("Separate each tag with SPACE.");

        ImGui::EndTable();
    }

    // list of anims
    constexpr auto table_flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("Animation Entries", 2, table_flags, {0.f, -FLT_MIN}))
    {
        ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthStretch, 0.4);
        ImGui::TableSetupColumn("Tags", ImGuiTableColumnFlags_WidthStretch, 0.6);
        ImGui::TableHeadersRow();

        auto anim_list = anim_manager->listAnims(filter_text, filter_mode);
        // std::ranges::sort(anim_list, {}, &AnimEntry::editor_id);

        ImGuiListClipper clipper;
        clipper.Begin(anim_list.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                auto edid = anim_list[row_n];

                ImGui::PushID(edid.data());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (anim_manager->hasCustomTags(edid))
                    ImGui::PushStyleColor(ImGuiCol_Text, {0.5f, 0.5f, 1.f, 1.f}); // indicate custom tags
                if (ImGui::Selectable(edid.data(), false))
                    testPlayPairedIdle(RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid));
                if (anim_manager->hasCustomTags(edid))
                    ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Click to test it on the nearest NPC.\n"
                                      "Best when in a good position and they are not attacking.\n"
                                      "The conditions are not checked. So be wary.");

                ImGui::TableNextColumn();
                auto tags_str = joinTags(*anim_manager->getTags(edid));
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputText("##", &tags_str, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (tags_str.empty())
                        anim_manager->clearTags(edid);
                    else
                        anim_manager->setTags(edid, splitTags(tags_str));
                }

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Press Enter to apply tag editing.\n"
                                      "The tags are sorted and seperated by SPACE.\n"
                                      "Leave empty and press Enter to set to default.\n"
                                      "(Remember to save to file afterwards.)");

                ImGui::PopID();
            }

        ImGui::EndTable();
    }
}

void drawCatMenu()
{
    // ImGui::ShowDemoWindow();

    if (ImGui::Begin("Kaputt Config Menu", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::SetWindowSize({600, 600}, ImGuiCond_FirstUseEver);

        if (ImGui::BeginTable("fileops", 4))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Config:");

            ImGui::TableNextColumn();
            if (ImGui::Button("Save", {-FLT_MIN, 0.f}))
                ;

            ImGui::TableNextColumn();
            if (ImGui::Button("Save As...", {-FLT_MIN, 0.f}))
                ;

            ImGui::TableNextColumn();
            if (ImGui::Button("Load...", {-FLT_MIN, 0.f}))
                ;
            ImGui::EndTable();
        }

        ImGui::Separator();

        ImGui::BeginChild("main", {0.f, -ImGui::GetFontSize() - 2.f});
        if (ImGui::BeginTabBar("##"))
        {
            if (ImGui::BeginTabItem("General")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Trigger")) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Filter"))
            {
                // drawFilterMenu();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Animation"))
            {
                drawAnimationMenu();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::TextDisabled(status_msg.c_str());
    }
    ImGui::End();
}
} // namespace kaputt