#include "menu.h"

#include "re.h"
#include "utils.h"
#include "kaputt.h"
#include "trigger.h"

#include <filesystem>
namespace fs = std::filesystem;

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

int filterFilename(ImGuiInputTextCallbackData* data)
{
    if (data->EventChar < 256 && (isalnum((char)data->EventChar) || ((char)data->EventChar == '_')))
        return 0;
    return 1;
}

void header(const char* label, int columns = 4)
{
    if (ImGui::BeginTable(label, columns))
    {
        ImGui::TableNextColumn();
        ImGui::Button(label, {-FLT_MIN, 0.f});
        ImGui::EndTable();
    }
}



void drawSettingMenu()
{
    auto  kaputt         = Kaputt::getSingleton();
    auto& precond_params = kaputt->precond_params;

    ImGui::SetNextItemOpen(true);
    if (ImGui::CollapsingHeader("Precondition"))
    {
        if (ImGui::BeginTable("big tbl", 4))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Essential Protection");
            ImGui::TableNextColumn();
            ImGui::RadioButton("enabled", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::ENABLED);
            ImGui::TableNextColumn();
            ImGui::RadioButton("protected", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::PROTECTED);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Player can still trigger killmoves on essential npcs.");
            ImGui::TableNextColumn();
            ImGui::RadioButton("disable", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::DISABLED);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Protected Protection");
            ImGui::TableNextColumn();
            ImGui::Checkbox(precond_params.protected_protection ? "enabled" : "disabled", &precond_params.protected_protection);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Hostile Range");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SliderFloat("##range", &precond_params.last_hostile_range, 0.f, 4096.f, "%.0f unit");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Hostile actors outside of this 'safe' range will be ignored.");
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("~= %.1f m", precond_params.last_hostile_range * 0.0142875f);
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("~= %.2f ft", precond_params.last_hostile_range * 0.046875f);

            ImGui::EndTable();
        }
        if (ImGui::BeginTable("smol tbl", 2))
        {
            ImGui::TableSetupColumn("1", 0, 1);
            ImGui::TableSetupColumn("2", 0, 3);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Skipped Races");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##Skipped Races", precond_params.skipped_race);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Races here won't participate in a killmove. Press Enter to apply changes.\n"
                                  "The default value is the vanilla setting, due to height, being a boss or other considerations.");

            ImGui::EndTable();
        }
    }
}

void drawTriggerMenu()
{
    auto post_trigger = PostHitTrigger::getSingleton();

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Vanilla"))
    {
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Post-Hit"))
    {
        ImGui::Checkbox("Enabled", &post_trigger->enabled);

        if (!post_trigger->enabled)
            ImGui::BeginDisabled();

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Checkbox("Bleedout Execution", &post_trigger->enable_bleedout_execution);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("One-hit killmove triggering on a bleeding out actor, even when the damage is not enough to kill.\n");

        if (ImGui::BeginTable("chances", 4))
        {
            ImGui::TableSetupColumn("Chances");
            ImGui::TableSetupColumn("Player->NPC");
            ImGui::TableSetupColumn("NPC->Player");
            ImGui::TableSetupColumn("NPC->NPC");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Killmove");
            for (auto i : {0, 1, 2})
            {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SliderFloat(fmt::format("##km{}", i).c_str(), &post_trigger->prob_km[i], 0.f, 1.f, "%.2f / 1.00");
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Execution");
            for (auto i : {0, 1, 2})
            {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SliderFloat(fmt::format("##exec{}", i).c_str(), &post_trigger->prob_exec[i], 0.f, 1.f, "%.2f / 1.00");
            }

            ImGui::EndTable();
        }

        if (!post_trigger->enabled)
            ImGui::EndDisabled();
    }
}

void drawAnimationMenu()
{
    static std::string filter_text = {};
    static int         filter_mode = 0; // 0 None 1 ID 2 Tags

    auto kaputt = Kaputt::getSingleton();

    // anim filters
    if (ImGui::BeginTable("filtertab", 4))
    {
        ImGui::TableSetupColumn("filter", ImGuiTableColumnFlags_WidthStretch, 0.5);
        ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch, 0.5 / 3);
        ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthStretch, 0.5 / 3);
        ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthStretch, 0.5 / 3);

        ImGui::TableNextColumn();
        ImGui::InputText("Filter by", &filter_text);

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
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        auto anim_list = kaputt->listAnims(filter_text, filter_mode);

        ImGuiListClipper clipper;
        clipper.Begin(anim_list.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                auto edid = anim_list[row_n];

                ImGui::PushID(edid.data());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (kaputt->anim_custom_tags_map.contains(edid))
                    ImGui::PushStyleColor(ImGuiCol_Text, {0.5f, 0.5f, 1.f, 1.f}); // indicate custom tags
                if (ImGui::Selectable(edid.data(), false))
                    testPlayPairedIdle(RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid));
                if (kaputt->anim_custom_tags_map.contains(edid))
                    ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Click to test it on the nearest NPC.\n"
                                      "Best when in a good position and they are not attacking.\n"
                                      "The conditions are not checked. So be wary.");

                ImGui::TableNextColumn();
                auto tags_str = joinTags(kaputt->getTags(edid));
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputText("##", &tags_str, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (tags_str.empty())
                        kaputt->anim_custom_tags_map.erase(edid);
                    else
                        kaputt->setTags(edid, splitTags(tags_str));
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

    // aiekick's style https://github.com/ocornut/imgui/issues/707#issuecomment-760219522
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.06f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.08f, 0.08f, 0.08f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.43f, 0.43f, 0.50f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.44f, 0.44f, 0.44f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.57f, 0.57f, 0.57f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.76f, 0.76f, 0.76f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.04f, 0.04f, 0.04f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.00f, 0.00f, 0.00f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.14f, 0.14f, 0.14f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.02f, 0.02f, 0.02f, 0.53f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.31f, 0.31f, 0.31f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.41f, 0.41f, 0.41f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.51f, 0.51f, 0.51f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.13f, 0.75f, 0.55f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.13f, 0.75f, 0.75f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.13f, 0.75f, 1.00f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.75f, 0.55f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.13f, 0.75f, 0.75f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.13f, 0.75f, 1.00f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.13f, 0.75f, 0.55f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.13f, 0.75f, 0.75f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.13f, 0.75f, 1.00f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.13f, 0.75f, 0.55f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.13f, 0.75f, 0.75f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.13f, 0.75f, 1.00f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0.13f, 0.75f, 0.55f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0.13f, 0.75f, 0.75f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.13f, 0.75f, 1.00f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.13f, 0.75f, 0.55f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.13f, 0.75f, 0.75f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.13f, 0.75f, 1.00f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(0.18f, 0.18f, 0.18f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(0.36f, 0.36f, 0.36f, 0.54f));
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.61f, 0.61f, 0.61f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, ImVec4(1.00f, 0.43f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.90f, 0.70f, 0.00f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImVec4(1.00f, 0.60f, 0.00f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.19f, 0.19f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.31f, 0.31f, 0.35f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.23f, 0.23f, 0.25f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(1.00f, 1.00f, 1.00f, 0.07f));
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.26f, 0.59f, 0.98f, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(1.00f, 1.00f, 0.00f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_NavWindowingHighlight, ImVec4(1.00f, 1.00f, 1.00f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_NavWindowingDimBg, ImVec4(0.80f, 0.80f, 0.80f, 0.20f));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.80f, 0.80f, 0.80f, 0.35f));

    auto kaputt = Kaputt::getSingleton();

    if (ImGui::Begin("Kaputt Config Menu", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::SetWindowSize({600, 600}, ImGuiCond_FirstUseEver);

        if (ImGui::BeginTable("fileops", 4))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Config:");

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Button, {0.5f, 0.1f, 0.1f, 1.f});
            if (ImGui::Button("Save", {-FLT_MIN, 0.f}))
                setStatusMessage(kaputt->saveConfig(def_config_path) ?
                                     fmt::format("Config saved to {}", def_config_path) :
                                     "Something went wrong while saving. Please check the log.");
            ImGui::PopStyleColor();

            ImGui::TableNextColumn();
            if (ImGui::Button("Save As Preset", {-FLT_MIN, 0.f}))
                ImGui::OpenPopup("save config");
            if (ImGui::BeginPopup("save config"))
            {
                static std::string save_name = {};
                if (ImGui::InputText("Press Enter", &save_name, ImGuiInputTextFlags_EnterReturnsTrue, filterFilename))
                {
                    setStatusMessage(
                        kaputt->saveConfig(config_dir + "\\"s + save_name + ".json") ?
                            "Config saved as " + save_name :
                            "Something went wrong while saving " + save_name + ". Please check the log.");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::TableNextColumn();
            if (ImGui::Button("Load Preset", {-FLT_MIN, 0.f}))
                ImGui::OpenPopup("load config");
            if (ImGui::BeginPopup("load config"))
            {
                static std::string load_name  = {};
                bool               has_preset = false;

                if (fs::exists(config_dir))
                    for (auto const& dir_entry : fs::directory_iterator{fs::path(config_dir)})
                        if (dir_entry.is_regular_file())
                            if (auto file_path = dir_entry.path(); file_path.extension() == ".json")
                            {
                                has_preset = true;
                                if (ImGui::Selectable(file_path.stem().string().c_str()))
                                {
                                    setStatusMessage(
                                        kaputt->loadConfig(file_path.string()) ?
                                            "Loaded config preset " + load_name :
                                            "Something went wrong while loading " + load_name + ". Please check the log.");
                                    ImGui::CloseCurrentPopup();
                                }
                            }

                if (!has_preset)
                    ImGui::TextDisabled("No presets found.");

                ImGui::EndPopup();
            }

            ImGui::EndTable();
        }

        ImGui::Separator();

        ImGui::BeginChild("main", {0.f, -ImGui::GetFontSize() - 2.f});
        if (ImGui::BeginTabBar("##"))
        {
            if (ImGui::BeginTabItem("Setting"))
            {
                drawSettingMenu();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Trigger"))
            {
                drawTriggerMenu();
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

    ImGui::PopStyleColor(53);
}
} // namespace kaputt