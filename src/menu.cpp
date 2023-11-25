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
    auto& required_refs  = kaputt->required_refs;
    auto& misc_params    = kaputt->misc_params;
    auto& tagging_params = kaputt->tagging_params;
    auto& precond_params = kaputt->precond_params;

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Misc"))
    {
        if (ImGui::BeginTable("misc1", 2))
        {
            ImGui::TableSetupColumn("1", 0, 1);
            ImGui::TableSetupColumn("2", 0, 3);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Debug Log");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox(misc_params.enable_debug_log ? "enabled##debug" : "disabled##debug", &misc_params.enable_debug_log))
            {
                auto level = misc_params.enable_debug_log ? spdlog::level::trace : spdlog::level::info;
                spdlog::set_level(level);
                spdlog::flush_on(level);
            }

            ImGui::EndTable();
        }

        if (ImGui::BeginTable("misc2", 4))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Disable Vanilla");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("killmove", &misc_params.disable_vanilla))
                required_refs.vanilla_killmove->value = !misc_params.disable_vanilla;
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("sneak kill", &misc_params.disable_vanilla_sneak))
                required_refs.vanilla_sneak->value = !misc_params.disable_vanilla_sneak;
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("dragon bite", &misc_params.disable_vanilla_dragon))
                required_refs.vanilla_dragon->value = !misc_params.disable_vanilla_dragon;

            ImGui::EndTable();
        }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
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
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Player can still trigger killmoves on essential npcs.");
            ImGui::TableNextColumn();
            ImGui::RadioButton("disable", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::DISABLED);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Protected Protection");
            ImGui::TableNextColumn();
            ImGui::Checkbox(precond_params.protected_protection ? "enabled##prot" : "disabled##prot", &precond_params.protected_protection);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Furniture Toggle");
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Toggle killmoves when victim is on certain types of furnitures.");
            ImGui::TableNextColumn();
            ImGui::Checkbox("sit", &precond_params.furn_sit);
            ImGui::TableNextColumn();
            ImGui::Checkbox("lean", &precond_params.furn_lean);
            ImGui::TableNextColumn();
            ImGui::Checkbox("sleep", &precond_params.furn_sleep);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Last Enemy Range");
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Hostile actors outside of this 'safe' range will be ignored.\n"
                                  "Set to 0 to effectively ignore any hostiles.");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SliderFloat("##range", &precond_params.last_hostile_range, 0.f, 4096.f, "%.0f unit");
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("~= %.1f m | %.2f ft", precond_params.last_hostile_range * 0.0142875f, precond_params.last_hostile_range * 0.046875f);
            ImGui::TableNextColumn();
            ImGui::Checkbox("player group only", &precond_params.last_hostile_player_follower_only);
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Only do this check on player and followers.\nUseful if you want some grand battle scene with many killmoves.");

            ImGui::EndTable();
        }
        if (ImGui::BeginTable("smol tbl", 2))
        {
            ImGui::TableSetupColumn("1", 0, 1);
            ImGui::TableSetupColumn("2", 0, 3);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Height Difference");
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("A vanilla check that restricts the difference of height (z coordinate) between attacker and victim.");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputFloat2("##height", precond_params.height_diff_range.data(), "%.1f");

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Excluded Races");
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Races here won't participate in a killmove.\n"
                                  "The default value is the vanilla setting, due to scale, being a boss or other considerations.");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##Skipped Races", precond_params.skipped_race);

            ImGui::EndTable();
        }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Animation Filtering"))
    {
        if (ImGui::BeginTable("tagger0", 2))
        {
            ImGui::TableSetupColumn("1", 0, 1);
            ImGui::TableSetupColumn("2", 0, 3);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("+Required Tags");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##reqtag", tagging_params.required_tags);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("+Banned Tags");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##bantag", tagging_params.banned_tags);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Player Decap");
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("disabled##playerdecap", &tagging_params.decap_disable_player))
                required_refs.decap_disable_player->value = tagging_params.decap_disable_player;
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("A RaceMenu bug crashes the game when beheading a character with face overlays.\n"
                                  "This bug mostly happens on players, hence this option if you still want to chop some other heads off.");

            ImGui::EndTable();
        }
        if (ImGui::BeginTable("tagger1", 4))
        {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Decap Perk");
            ImGui::TableNextColumn();
            if (ImGui::RadioButton("required", tagging_params.decap_requires_perk && !tagging_params.decap_bleed_ignore_perk))
            {
                required_refs.decap_requires_perk->value = tagging_params.decap_requires_perk = true;
                required_refs.decap_bleed_ignore_perk->value = tagging_params.decap_bleed_ignore_perk = false;
            }
            ImGui::TableNextColumn();
            if (ImGui::RadioButton("bleedout ignored", tagging_params.decap_requires_perk && tagging_params.decap_bleed_ignore_perk))
            {
                required_refs.decap_requires_perk->value = tagging_params.decap_requires_perk = true;
                required_refs.decap_bleed_ignore_perk->value = tagging_params.decap_bleed_ignore_perk = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Bleedout killmoves ignores perk requirement.");
            ImGui::TableNextColumn();
            if (ImGui::RadioButton("ignored", !tagging_params.decap_requires_perk))
                required_refs.decap_requires_perk->value = tagging_params.decap_requires_perk = false;

            ImGui::EndTable();
        }
        if (ImGui::BeginTable("tagger2", 3))
        {
            ImGui::TableSetupColumn("1", 0, 1);
            ImGui::TableSetupColumn("2", 0, 1);
            ImGui::TableSetupColumn("3", 0, 2);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Decap Chance");

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("use chance", &tagging_params.decap_use_chance))
                required_refs.decap_use_chance->value = tagging_params.decap_use_chance;
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Don't determine decap chance explicitly.\n"
                                  "Sometimes there aren't proper decap animations for the scene. Disabling this makes non-decap animations still playable.");

            if (!tagging_params.decap_use_chance)
                ImGui::BeginDisabled();

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::SliderFloat("##Decap Chance", &tagging_params.decap_percent, 0.f, 100.f, "%.0f %%"))
                required_refs.decap_percent->value = tagging_params.decap_percent;

            if (!tagging_params.decap_use_chance)
                ImGui::EndDisabled();

            ImGui::EndTable();
        }
    }
}

void drawTriggerMenu()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Vanilla-ish"))
    {
        ImGui::Indent();
        ImGui::PushID("vanilla");

        auto vanilla_trigger = VanillaTrigger::getSingleton();

        ImGui::Checkbox("Enabled", &vanilla_trigger->enabled);
        ImGui::SameLine();
        if (ImGui::BeginTable("desc", 1, ImGuiTableFlags_Borders))
        {
            ImGui::TableNextColumn();
            ImGui::Text("Triggers when an supposedly lethal attack is initiated.");
            ImGui::EndTable();
        }

        if (!vanilla_trigger->enabled)
            ImGui::BeginDisabled();

        if (ImGui::BeginTable("exec", 3))
        {
            ImGui::TableNextColumn();
            ImGui::Checkbox("Bleedout Execution", &vanilla_trigger->enable_bleedout_execution);
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Triggering on a bleeding out actor, even when the damage is not enough to kill.\n");

            ImGui::TableNextColumn();
            ImGui::Checkbox("Get Up Execution", &vanilla_trigger->enable_getup_execution);
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Triggering on an actor recovering from ragdoll, even when the damage is not enough to kill.\n"
                                  "Ragdoll executions are disabled due to them being too buggy to handle.");

            ImGui::EndTable();
        }

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
                ImGui::SliderFloat(fmt::format("##km{}", i).c_str(), &vanilla_trigger->prob_km[i], 0.f, 100.f, "%.0f %%");
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Execution");
            for (auto i : {0, 1, 2})
            {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SliderFloat(fmt::format("##exec{}", i).c_str(), &vanilla_trigger->prob_exec[i], 0.f, 100.f, "%.0f %%");
            }

            ImGui::EndTable();
        }

        if (!vanilla_trigger->enabled)
            ImGui::EndDisabled();

        ImGui::Unindent();
        ImGui::PopID();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Post-Hit"))
    {
        ImGui::Indent();
        ImGui::PushID("posthit");

        auto post_trigger = PostHitTrigger::getSingleton();

        ImGui::Checkbox("Enabled", &post_trigger->enabled);
        ImGui::SameLine();
        if (ImGui::BeginTable("desc", 1, ImGuiTableFlags_Borders))
        {
            ImGui::TableNextColumn();
            ImGui::Text("Triggers after a lethal hit landed.");
            ImGui::EndTable();
        }

        if (!post_trigger->enabled)
            ImGui::BeginDisabled();

        if (ImGui::BeginTable("exec", 3))
        {
            ImGui::TableNextColumn();
            ImGui::Checkbox("Bleedout Execution", &post_trigger->enable_bleedout_execution);
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("One-hit triggering on a bleeding out actor, even when the damage is not enough to kill.\n");

            ImGui::TableNextColumn();
            ImGui::Checkbox("Get Up Execution", &post_trigger->enable_getup_execution);
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("One-hit triggering on an actor recovering from ragdoll, even when the damage is not enough to kill.\n"
                                  "Ragdoll executions are disabled due to them being too buggy to handle.");

            ImGui::TableNextColumn();
            ImGui::Checkbox("Execution Instakill", &post_trigger->instakill);
            ImGui::SameLine();
            ImGui::TextDisabled("[?]");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Kills victim instantly even if the execution animation somehow didn't play.");

            ImGui::EndTable();
        }

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
                ImGui::SliderFloat(fmt::format("##km{}", i).c_str(), &post_trigger->prob_km[i], 0.f, 100.f, "%.0f %%");
            }

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Execution");
            for (auto i : {0, 1, 2})
            {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::SliderFloat(fmt::format("##exec{}", i).c_str(), &post_trigger->prob_exec[i], 0.f, 100.f, "%.0f %%");
            }

            ImGui::EndTable();
        }

        if (!post_trigger->enabled)
            ImGui::EndDisabled();

        ImGui::Unindent();
        ImGui::PopID();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Sneak Keypress"))
    {
        ImGui::PushID("sneak");
        ImGui::Indent();

        auto sneak_trigger = SneakTrigger::getSingleton();

        ImGui::Checkbox("Enabled", &sneak_trigger->enabled);
        ImGui::SameLine();
        if (ImGui::BeginTable("desc", 1, ImGuiTableFlags_Borders))
        {
            ImGui::TableNextColumn();
            ImGui::Text("Triggers sneak killmoves on the crosshair target with a key press.");
            ImGui::EndTable();
        }

        if (!sneak_trigger->enabled)
            ImGui::BeginDisabled();

        if (ImGui::BeginTable("key", 2))
        {
            ImGui::TableNextColumn();
            ImGui::InputScalar("Key (Scancode)", ImGuiDataType_U32, &sneak_trigger->key_scancode);

            ImGui::TableNextColumn();
            if (ImGui::BeginTable("key name", 1, ImGuiTableFlags_Borders))
            {
                ImGui::TableNextColumn();
                ImGui::Text(scanCode2String(sneak_trigger->key_scancode).c_str());
                ImGui::EndTable();
            }
            ImGui::EndTable();
        }

        ImGui::Checkbox("Need Crouching", &sneak_trigger->need_crouch);
        ImGui::SameLine();
        ImGui::TextDisabled("[?]");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("If disabled, you can trigger this while standing as long as you are not detected.\n"
                              "The animation will be different though.");

        if (!sneak_trigger->enabled)
            ImGui::EndDisabled();

        ImGui::PopID();
        ImGui::Unindent();
    }
}

void drawAnimationMenu()
{
    static std::string filter_text = {};
    static int         filter_mode = 0; // 0 None 1 ID 2 Tags

    auto  kaputt      = Kaputt::getSingleton();
    auto& tagexp_list = kaputt->tagexp_list;

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
                              "Tags will be expanded only once i.e. the tags on the right cannot be expanded furthermore.");

        ImGui::TableNextColumn();
        if (ImGui::Button("Add", {-FLT_MIN, 0.f}))
            tagexp_list.try_emplace("from", StrSet{"to"});

        ImGui::EndTable();
    }

    if (ImGui::BeginTable("tagexp", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                          {0.f, (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2) * 5}))
    {
        ImGui::TableSetupColumn("from", ImGuiTableColumnFlags_WidthStretch, 0.2f);
        ImGui::TableSetupColumn("arrow", ImGuiTableColumnFlags_WidthStretch, 0.05f);
        ImGui::TableSetupColumn("to", ImGuiTableColumnFlags_WidthStretch, 0.75f);

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
                                  "Leave this empty and press Enter to delete the item.");

            ImGui::TableNextColumn();
            ImGui::Text("->");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##to", to);

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


    // anim filters
    if (ImGui::BeginTable("filtertab", 4))
    {
        ImGui::TableSetupColumn("filter", ImGuiTableColumnFlags_WidthStretch, 0.5);
        ImGui::TableSetupColumn("1", ImGuiTableColumnFlags_WidthStretch, 0.5f / 3);
        ImGui::TableSetupColumn("2", ImGuiTableColumnFlags_WidthStretch, 0.5f / 3);
        ImGui::TableSetupColumn("3", ImGuiTableColumnFlags_WidthStretch, 0.5f / 3);

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
        ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Tags", ImGuiTableColumnFlags_WidthStretch, 0.6f);
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