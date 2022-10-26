#include "menu.h"
#include "animation.h"
#include "filter.h"

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

std::optional<std::string_view> pickARulePopupContent()
{
    for (auto name : Rule::listRules())
        if (ImGui::Selectable(name.data()))
            return name;
    return std::nullopt;
}


void drawFilterMenu()
{
    static size_t selected_tagexp_idx = INT64_MAX;
    static size_t selected_tagger_idx = INT64_MAX;

    auto  filter_pipeline = FilterPipeline::getSingleton();
    auto& tagexp_list     = filter_pipeline->tagexp_list;
    auto& tagger_list     = filter_pipeline->tagger_list;

    // File operations
    if (ImGui::BeginTable("io", 3))
    {
        ImGui::TableNextColumn();
        if (ImGui::Button("Save Filter", {-FLT_MIN, 0.f}))
        {
            filter_pipeline->saveDefaultFile();
            setStatusMessage("Filter saved to default.toml");
        }

        ImGui::TableNextColumn();
        ImGui::Button("Save Filter As...", {-FLT_MIN, 0.f});
        ImGui::TableNextColumn();
        ImGui::Button("Load Filter From...", {-FLT_MIN, 0.f});
        ImGui::EndTable();
    }
    ImGui::Separator();

    // Tag Expansions
    if (ImGui::BeginTable("tagexp config", 3))
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
                              "Click the '->' to select the item for removal.");

        ImGui::TableNextColumn();
        if (ImGui::Button("Add", {-FLT_MIN, 0.f}))
            tagexp_list.push_back({});

        ImGui::TableNextColumn();
        if (ImGui::Button("Remove", {-FLT_MIN, 0.f}) && (selected_tagexp_idx < tagexp_list.size()))
            tagexp_list.erase(tagexp_list.begin() + selected_tagexp_idx);

        ImGui::EndTable();
    }


    if (ImGui::BeginTable("tagexp", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                          {0.f, (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2) * 5}))
    {
        ImGui::TableSetupColumn("from", ImGuiTableColumnFlags_WidthStretch, 0.2);
        ImGui::TableSetupColumn("arrow", ImGuiTableColumnFlags_WidthStretch, 0.05);
        ImGui::TableSetupColumn("to", ImGuiTableColumnFlags_WidthStretch, 0.75);

        ImGuiListClipper clipper;
        clipper.Begin(tagexp_list.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                auto& tagexp = tagexp_list[row_n];

                ImGui::PushID(row_n);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##from", &tagexp.from);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::Selectable("->", selected_tagexp_idx == row_n))
                    selected_tagexp_idx = row_n;

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                drawTagsInputText("##to", tagexp.to);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Press Enter to apply.\n"
                                      "The tags are sorted and seperated by SPACE.");

                ImGui::PopID();
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
                tagger_list.push_back({});
                tagger_list.back().rule = Rule::getRule(result.value());
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
            ImGui::AlignTextToFramePadding();
            ImGui::PushStyleColor(ImGuiCol_Text, {0.5f, 0.5f, 1.f, 1.f});
            if (ImGui::Selectable(tagger.rule->getName().data(), selected_tagger_idx == count))
                selected_tagger_idx = count;
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(tagger.rule->getHint().data());
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##", "Description", &tagger.comment);

            ImGui::TableNextColumn();
            tagger.rule->drawParams();

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

    auto anim_manager = AnimEntryManager::getSingleton();

    // file operation
    if (ImGui::BeginTable("filetab", 3))
    {
        ImGui::TableNextColumn();
        if (ImGui::Button("Save Custom Tags", {-FLT_MIN, 0.f}))
        {
            anim_manager->saveCustomFile(anim_manager->getDefaultCustomFilePath());
            setStatusMessage("Custom tags saved!");
        }
        ImGui::TableNextColumn();
        if (ImGui::Button("Reload Custom Tags", {-FLT_MIN, 0.f}))
        {
            anim_manager->loadCustomFile(anim_manager->getDefaultCustomFilePath());
            setStatusMessage("Custom tags reloaded!");
        }
        ImGui::TableNextColumn();
        if (ImGui::Button("Clear Custom Tags", {-FLT_MIN, 0.f}))
        {
            anim_manager->clearCustomTags();
            setStatusMessage("Custom tags cleared!");
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Remember to save to file.");

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

        // filter
        std::vector<AnimEntry*> anim_list;
        for (auto& [edid, anim] : anim_manager->anim_dict)
        {
            if ((filter_mode == 1) && !edid.contains(filter_text)) continue;
            if ((filter_mode == 2) &&
                std::ranges::none_of(splitTags(filter_text), [&](const std::string& tag) { return anim.getTags().contains(tag); }))
                continue;
            anim_list.push_back(&anim);
        }
        // sort
        std::ranges::sort(anim_list, {}, &AnimEntry::editor_id);

        ImGuiListClipper clipper;
        clipper.Begin(anim_list.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                auto& anim = *anim_list[row_n];
                auto& edid = anim.editor_id;

                ImGui::PushID(edid.c_str());

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (anim.custom_tags.has_value())
                    ImGui::PushStyleColor(ImGuiCol_Text, {0.5f, 0.5f, 1.f, 1.f}); // indicate custom tags
                if (ImGui::Selectable(edid.c_str(), false))
                    anim.testPlay();
                if (anim.custom_tags.has_value())
                    ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Click to test it on the nearest NPC.\n"
                                      "Best when in a good position and they are not attacking.\n"
                                      "The conditions are not checked. So be wary.");

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
    ImGui::ShowDemoWindow();

    if (ImGui::Begin("Kaputt Config Menu", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::SetWindowSize({600, 600}, ImGuiCond_FirstUseEver);

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
}
} // namespace kaputt