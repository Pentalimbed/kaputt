#include "menu.h"

#include "re.h"
#include "utils.h"
#include "animation.h"
#include "filter.h"

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

std::optional<std::string_view> pickARulePopupContent()
{
    for (auto& [name, _] : getRule())
        if (ImGui::Selectable(name.data()))
            return name;
    return std::nullopt;
}

void drawFilterMenu()
{
    static size_t selected_tagger_idx = INT64_MAX;

    auto  filter_pipeline = FilterPipeline::getSingleton();
    auto& tagexp_list     = filter_pipeline->tagexp_list;
    auto& tagger_list     = filter_pipeline->tagger_list;

    // Tag Expansions
    if (ImGui::BeginTable("tagexp config", 2))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Tag Expansion");
        ImGui::AlignTextToFramePadding();
        ImGui::SameLine();
        ImGui::TextDisabled("[?]");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("If an animation has the tag on the left, then all tags on the right are provided.\n"
                              "Tags will be expanded only once i.e. the tags on the right cannot be expanded furthermore.\n"
                              "Click the '->' to select the item for removal. Duplicate items will be removed after save and reload.");

        ImGui::TableNextColumn();
        if (ImGui::Button("Add", {-FLT_MIN, 0.f}))
            tagexp_list.try_emplace("from", StrSet{"to"});

        ImGui::EndTable();
    }

    if (ImGui::BeginTable("tagexp", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                          {0.f, (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2) * 5}))
    {
        ImGui::TableSetupColumn("from", ImGuiTableColumnFlags_WidthStretch, 0.2);
        ImGui::TableSetupColumn("arrow", ImGuiTableColumnFlags_WidthStretch, 0.05);
        ImGui::TableSetupColumn("to", ImGuiTableColumnFlags_WidthStretch, 0.75);

        std::string swap_from = {}, swap_to = {};
        for (auto& [from, to] : tagexp_list)
        {
            ImGui::PushID(from.c_str());

            ImGui::TableNextColumn();
            std::string temp_from = from;
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##from", &temp_from, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                swap_from = from;
                swap_to   = temp_from;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Press Enter to apply. It will be sorted.\n"
                                  "If the tag already exists, nothing will happen.\n"
                                  "Leave empty and press Enter to delete the item.");

            ImGui::TableNextColumn();
            ImGui::Text("->");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##to", to);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Press Enter to apply.\n"
                                  "The tags are sorted and seperated by SPACE.");

            ImGui::PopID();
        }
        if (!swap_from.empty() && !tagexp_list.contains(swap_to))
        {
            if (swap_to.empty())
                tagexp_list.erase(swap_from);
            else
            {
                auto node  = tagexp_list.extract(swap_from);
                node.key() = swap_to;
                tagexp_list.insert(std::move(node));
            }
        }

        ImGui::EndTable();
    }

    // Taggers
    if (ImGui::BeginTable("tagger config", 5))
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Tagging Rules");
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("[?]");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Each rule, if condition is met, will provide some required tags and banned tags.\n"
                              "An animation can be selected if it has all required tags and none of banned tags.");

        ImGui::TableNextColumn();
        if (ImGui::Button("Add", {-FLT_MIN, 0.f}))
            ImGui::OpenPopup("Pick A Rule");
        if (ImGui::BeginPopup("Pick A Rule"))
        {
            auto result = pickARulePopupContent();
            if (result.has_value())
            {
                tagger_list.push_back(Tagger{.rule = {result.value()}});
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::TableNextColumn();
        if (ImGui::Button("Remove", {-FLT_MIN, 0.f}) && (selected_tagger_idx < tagger_list.size()))
            tagger_list.erase(tagger_list.begin() + selected_tagger_idx);

        ImGui::TableNextColumn();
        if (ImGui::Button("Move Up", {-FLT_MIN, 0.f}) && (selected_tagger_idx < tagger_list.size()) && (selected_tagger_idx > 0))
            std::swap(tagger_list[selected_tagger_idx], tagger_list[selected_tagger_idx - 1]), --selected_tagger_idx;

        ImGui::TableNextColumn();
        if (ImGui::Button("Move Down", {-FLT_MIN, 0.f}) && (selected_tagger_idx < tagger_list.size() - 1))
            std::swap(tagger_list[selected_tagger_idx], tagger_list[selected_tagger_idx + 1]), ++selected_tagger_idx;

        ImGui::EndTable();
    }

    if (ImGui::BeginTable("tagger", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Rule", ImGuiTableColumnFlags_WidthStretch, 0.4);
        ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthStretch, 0.3);
        ImGui::TableSetupColumn("Tags", ImGuiTableColumnFlags_WidthStretch, 0.3);
        ImGui::TableHeadersRow();

        size_t count = 0;
        for (auto& tagger : tagger_list)
        {
            ImGui::PushID(count);

            ImGui::TableNextColumn();
            ImGui::Checkbox("##enable", &tagger.rule.enabled);
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::PushStyleColor(ImGuiCol_Text, tagger.rule.enabled ? ImVec4{0.5f, 0.5f, 1.f, 1.f} : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            if (ImGui::Selectable(tagger.rule.type.c_str(), selected_tagger_idx == count))
                selected_tagger_idx = count;
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(getRule().at(tagger.rule.type)->getHint().data());
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##comment", "Comment", &tagger.rule.comment);

            ImGui::TableNextColumn();
            tagger.rule.drawParams();

            ImGui::TableNextColumn();
            ImGui::Checkbox("If True", &tagger.enable_true);
            if (tagger.enable_true)
            {
                ImGui::PushID(1);
                drawTagsInputText("Require", tagger.true_tags.required_tags);
                drawTagsInputText("Ban", tagger.true_tags.banned_tags);
                ImGui::PopID();
            }
            ImGui::Checkbox("If False", &tagger.enable_false);
            if (tagger.enable_false)
            {
                ImGui::PushID(0);
                drawTagsInputText("Require", tagger.false_tags.required_tags);
                drawTagsInputText("Ban", tagger.false_tags.banned_tags);
                ImGui::PopID();
            }

            ImGui::PopID();
            ++count;
        }

        ImGui::EndTable();
    }
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
                drawFilterMenu();
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