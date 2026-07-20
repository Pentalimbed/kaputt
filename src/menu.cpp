#include "menu.h"

#include "re.h"
#include "utils.h"
#include "kaputt.h"
#include "trigger.h"
#include "FUCK_API.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace kaputt
{
static std::mutex  status_msg_mutex;
static std::string status_msg = "Kaputt Ver. " + SKSE::PluginDeclaration::GetSingleton()->GetVersion().string();
void               setStatusMessage(std::string_view msg)
{
    std::scoped_lock l(status_msg_mutex);
    status_msg = msg;
}

void sanitizePresetName(std::string& name)
{
    std::erase_if(name, [](unsigned char c) { return !std::isalnum(c) && c != '_'; });
}

bool radioButton(const char* label, bool selected)
{
    return FUCK::Selectable(label, selected);
}

bool radioButton(const char* label, int* value, int button_value)
{
    if (FUCK::Selectable(label, *value == button_value))
    {
        *value = button_value;
        return true;
    }
    return false;
}

void header(const char* label, int columns = 4)
{
    if (FUCK::BeginTable(label, columns))
    {
        FUCK::TableNextColumn();
        FUCK::Button(label);
        FUCK::EndTable();
    }
}



void drawSettingMenu()
{
    auto  kaputt         = Kaputt::getSingleton();
    auto& required_refs  = kaputt->required_refs;
    auto& misc_params    = kaputt->misc_params;
    auto& tagging_params = kaputt->tagging_params;
    auto& precond_params = kaputt->precond_params;

    FUCK::SetNextItemOpen(true, ImGuiCond_Once);
    if (FUCK::CollapsingHeader("Misc"))
    {
        if (FUCK::BeginTable("misc1", 2, FUCK::TableFlags::kSizingStretchProp))
        {
            FUCK::TableSetupColumn("1", FUCK::TableColumnFlags::kNone, 1.0f);
            FUCK::TableSetupColumn("2", FUCK::TableColumnFlags::kNone, 3.0f);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Debug Log");
            FUCK::TableNextColumn();
            if (FUCK::Checkbox(misc_params.enable_debug_log ? "enabled##debug" : "disabled##debug", &misc_params.enable_debug_log))
            {
                auto level = misc_params.enable_debug_log ? spdlog::level::trace : spdlog::level::info;
                spdlog::set_level(level);
                spdlog::flush_on(level);
            }

            FUCK::EndTable();
        }

        if (FUCK::BeginTable("misc2", 4))
        {
            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Disable Vanilla");
            FUCK::TableNextColumn();
            if (FUCK::Checkbox("killmove", &misc_params.disable_vanilla))
                required_refs.vanilla_killmove->value = !misc_params.disable_vanilla;
            FUCK::TableNextColumn();
            if (FUCK::Checkbox("sneak kill", &misc_params.disable_vanilla_sneak))
                required_refs.vanilla_sneak->value = !misc_params.disable_vanilla_sneak;
            FUCK::TableNextColumn();
            if (FUCK::Checkbox("dragon bite", &misc_params.disable_vanilla_dragon))
                required_refs.vanilla_dragon->value = !misc_params.disable_vanilla_dragon;

            FUCK::EndTable();
        }
    }

    FUCK::SetNextItemOpen(true, ImGuiCond_Once);
    if (FUCK::CollapsingHeader("Precondition"))
    {
        if (FUCK::BeginTable("big tbl", 4))
        {
            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Essential Protection");
            FUCK::TableNextColumn();
            radioButton("enabled", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::ENABLED);
            FUCK::TableNextColumn();
            radioButton("protected", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::PROTECTED);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Player can still trigger killmoves on essential npcs.");
            FUCK::TableNextColumn();
            radioButton("disable", (int*)&precond_params.essential_protection, (int)PreconditionParams::ESSENTIAL_PROT_ENUM::DISABLED);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Protected Protection");
            FUCK::TableNextColumn();
            FUCK::Checkbox(precond_params.protected_protection ? "enabled##prot" : "disabled##prot", &precond_params.protected_protection);
            FUCK::TableNextRow();

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Furniture Toggle");
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Toggle killmoves when victim is on certain types of furnitures.");
            FUCK::TableNextColumn();
            FUCK::Checkbox("sit", &precond_params.furn_sit);
            FUCK::TableNextColumn();
            FUCK::Checkbox("lean", &precond_params.furn_lean);
            FUCK::TableNextColumn();
            FUCK::Checkbox("sleep", &precond_params.furn_sleep);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Last Enemy Range");
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Hostile actors outside of this 'safe' range will be ignored.\n"
                                 "Set to 0 to effectively ignore any hostiles.");
            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            FUCK::SliderFloat("##range", &precond_params.last_hostile_range, 0.f, 4096.f, "%.0f unit");
            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("~= %.1f m | %.2f ft", precond_params.last_hostile_range * 0.0142875f, precond_params.last_hostile_range * 0.046875f);
            FUCK::TableNextColumn();
            FUCK::Checkbox("player group only", &precond_params.last_hostile_player_follower_only);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Only do this check on player and followers.\nUseful if you want some grand battle scene with many killmoves.");

            FUCK::EndTable();
        }
        if (FUCK::BeginTable("smol tbl", 2, FUCK::TableFlags::kSizingStretchProp))
        {
            FUCK::TableSetupColumn("1", FUCK::TableColumnFlags::kNone, 1.0f);
            FUCK::TableSetupColumn("2", FUCK::TableColumnFlags::kNone, 3.0f);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Height Difference");
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("A vanilla check that restricts the difference of height (z coordinate) between attacker and victim.");
            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            FUCK::DragFloat2("##height", precond_params.height_diff_range.data(), 0.1f, 0.0f, 0.0f, "%.1f");

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Excluded Races");
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Races here won't participate in a killmove.\n"
                                 "The default value is the vanilla setting, due to scale, being a boss or other considerations.");
            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##Skipped Races", precond_params.skipped_race);

            FUCK::EndTable();
        }
    }

    FUCK::SetNextItemOpen(true, ImGuiCond_Once);
    if (FUCK::CollapsingHeader("Animation Filtering"))
    {
        if (FUCK::BeginTable("tagger0", 2, FUCK::TableFlags::kSizingStretchProp))
        {
            FUCK::TableSetupColumn("1", FUCK::TableColumnFlags::kNone, 1.0f);
            FUCK::TableSetupColumn("2", FUCK::TableColumnFlags::kNone, 3.0f);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("+Required Tags");
            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##reqtag", tagging_params.required_tags);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("+Banned Tags");
            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##bantag", tagging_params.banned_tags);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Player Decap");
            FUCK::TableNextColumn();
            if (FUCK::Checkbox("disabled##playerdecap", &tagging_params.decap_disable_player))
                required_refs.decap_disable_player->value = tagging_params.decap_disable_player;
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("A RaceMenu bug crashes the game when beheading a character with face overlays.\n"
                                 "This bug mostly happens on players, hence this option if you still want to chop some other heads off.");

            FUCK::EndTable();
        }
        if (FUCK::BeginTable("tagger1", 4))
        {
            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Decap Perk");
            FUCK::TableNextColumn();
            if (radioButton("required", tagging_params.decap_requires_perk && !tagging_params.decap_bleed_ignore_perk))
            {
                required_refs.decap_requires_perk->value = tagging_params.decap_requires_perk = true;
                required_refs.decap_bleed_ignore_perk->value = tagging_params.decap_bleed_ignore_perk = false;
            }
            FUCK::TableNextColumn();
            if (radioButton("bleedout ignored", tagging_params.decap_requires_perk && tagging_params.decap_bleed_ignore_perk))
            {
                required_refs.decap_requires_perk->value = tagging_params.decap_requires_perk = true;
                required_refs.decap_bleed_ignore_perk->value = tagging_params.decap_bleed_ignore_perk = true;
            }
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Bleedout killmoves ignores perk requirement.");
            FUCK::TableNextColumn();
            if (radioButton("ignored", !tagging_params.decap_requires_perk))
                required_refs.decap_requires_perk->value = tagging_params.decap_requires_perk = false;

            FUCK::EndTable();
        }
        if (FUCK::BeginTable("tagger2", 3, FUCK::TableFlags::kSizingStretchProp))
        {
            FUCK::TableSetupColumn("1", FUCK::TableColumnFlags::kNone, 1.0f);
            FUCK::TableSetupColumn("2", FUCK::TableColumnFlags::kNone, 1.0f);
            FUCK::TableSetupColumn("3", FUCK::TableColumnFlags::kNone, 2.0f);

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Decap Chance");

            FUCK::TableNextColumn();
            if (FUCK::Checkbox("use chance", &tagging_params.decap_use_chance))
                required_refs.decap_use_chance->value = tagging_params.decap_use_chance;
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Don't determine decap chance explicitly.\n"
                                 "Sometimes there aren't proper decap animations for the scene. Disabling this makes non-decap animations still playable.");

            if (!tagging_params.decap_use_chance)
                FUCK::BeginDisabled();

            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            if (FUCK::SliderFloat("##Decap Chance", &tagging_params.decap_percent, 0.f, 100.f, "%.0f %%"))
                required_refs.decap_percent->value = tagging_params.decap_percent;

            if (!tagging_params.decap_use_chance)
                FUCK::EndDisabled();

            FUCK::EndTable();
        }
    }
}

void drawTriggerMenu()
{
    FUCK::SetNextItemOpen(true, ImGuiCond_Once);
    if (FUCK::CollapsingHeader("Vanilla-ish"))
    {
        FUCK::Indent();
        FUCK::PushID("vanilla");

        auto vanilla_trigger = VanillaTrigger::getSingleton();

        FUCK::Checkbox("Enabled", &vanilla_trigger->enabled);
        FUCK::SameLine();
        if (FUCK::BeginTable("desc", 1, FUCK::TableFlags::kBorders))
        {
            FUCK::TableNextColumn();
            FUCK::Text("Triggers when an supposedly lethal attack is initiated.");
            FUCK::EndTable();
        }

        if (!vanilla_trigger->enabled)
            FUCK::BeginDisabled();

        if (FUCK::BeginTable("exec", 3))
        {
            FUCK::TableNextColumn();
            FUCK::Checkbox("Bleedout Execution", &vanilla_trigger->enable_bleedout_execution);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Triggering on a bleeding out actor, even when the damage is not enough to kill.\n");

            FUCK::TableNextColumn();
            FUCK::Checkbox("Get Up Execution", &vanilla_trigger->enable_getup_execution);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Triggering on an actor recovering from ragdoll, even when the damage is not enough to kill.\n"
                                 "Ragdoll executions are disabled due to them being too buggy to handle.");

            FUCK::EndTable();
        }

        if (FUCK::BeginTable("chances", 4))
        {
            FUCK::TableSetupColumn("Chances");
            FUCK::TableSetupColumn("Player->NPC");
            FUCK::TableSetupColumn("NPC->Player");
            FUCK::TableSetupColumn("NPC->NPC");
            FUCK::TableHeadersRow();

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Killmove");
            for (auto i : {0, 1, 2})
            {
                FUCK::TableNextColumn();
                FUCK::SetNextItemWidth(-FLT_MIN);
                FUCK::SliderFloat(std::format("##km{}", i).c_str(), &vanilla_trigger->prob_km[i], 0.f, 100.f, "%.0f %%");
            }

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Execution");
            for (auto i : {0, 1, 2})
            {
                FUCK::TableNextColumn();
                FUCK::SetNextItemWidth(-FLT_MIN);
                FUCK::SliderFloat(std::format("##exec{}", i).c_str(), &vanilla_trigger->prob_exec[i], 0.f, 100.f, "%.0f %%");
            }

            FUCK::EndTable();
        }

        if (!vanilla_trigger->enabled)
            FUCK::EndDisabled();

        FUCK::Unindent();
        FUCK::PopID();
    }

    FUCK::SetNextItemOpen(true, ImGuiCond_Once);
    if (FUCK::CollapsingHeader("Post-Hit"))
    {
        FUCK::Indent();
        FUCK::PushID("posthit");

        auto post_trigger = PostHitTrigger::getSingleton();

        FUCK::Checkbox("Enabled", &post_trigger->enabled);
        FUCK::SameLine();
        if (FUCK::BeginTable("desc", 1, FUCK::TableFlags::kBorders))
        {
            FUCK::TableNextColumn();
            FUCK::Text("Triggers after a lethal hit landed.");
            FUCK::EndTable();
        }

        if (!post_trigger->enabled)
            FUCK::BeginDisabled();

        if (FUCK::BeginTable("exec", 3))
        {
            FUCK::TableNextColumn();
            FUCK::Checkbox("Bleedout Execution", &post_trigger->enable_bleedout_execution);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("One-hit triggering on a bleeding out actor, even when the damage is not enough to kill.\n");

            FUCK::TableNextColumn();
            FUCK::Checkbox("Get Up Execution", &post_trigger->enable_getup_execution);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("One-hit triggering on an actor recovering from ragdoll, even when the damage is not enough to kill.\n"
                                 "Ragdoll executions are disabled due to them being too buggy to handle.");

            FUCK::TableNextColumn();
            FUCK::Checkbox("Execution Instakill", &post_trigger->instakill);
            FUCK::SameLine();
            FUCK::TextDisabled("[?]");
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Kills victim instantly even if the execution animation somehow didn't play.");

            FUCK::EndTable();
        }

        if (FUCK::BeginTable("chances", 4))
        {
            FUCK::TableSetupColumn("Chances");
            FUCK::TableSetupColumn("Player->NPC");
            FUCK::TableSetupColumn("NPC->Player");
            FUCK::TableSetupColumn("NPC->NPC");
            FUCK::TableHeadersRow();

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Killmove");
            for (auto i : {0, 1, 2})
            {
                FUCK::TableNextColumn();
                FUCK::SetNextItemWidth(-FLT_MIN);
                FUCK::SliderFloat(std::format("##km{}", i).c_str(), &post_trigger->prob_km[i], 0.f, 100.f, "%.0f %%");
            }

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            FUCK::Text("Execution");
            for (auto i : {0, 1, 2})
            {
                FUCK::TableNextColumn();
                FUCK::SetNextItemWidth(-FLT_MIN);
                FUCK::SliderFloat(std::format("##exec{}", i).c_str(), &post_trigger->prob_exec[i], 0.f, 100.f, "%.0f %%");
            }

            FUCK::EndTable();
        }

        if (!post_trigger->enabled)
            FUCK::EndDisabled();

        FUCK::Unindent();
        FUCK::PopID();
    }

    FUCK::SetNextItemOpen(true, ImGuiCond_Once);
    if (FUCK::CollapsingHeader("Sneak Keypress"))
    {
        FUCK::PushID("sneak");
        FUCK::Indent();

        auto sneak_trigger = SneakTrigger::getSingleton();

        FUCK::Checkbox("Enabled", &sneak_trigger->enabled);
        FUCK::SameLine();
        if (FUCK::BeginTable("desc", 1, FUCK::TableFlags::kBorders))
        {
            FUCK::TableNextColumn();
            FUCK::Text("Triggers sneak killmoves on the crosshair target with a key press.");
            FUCK::EndTable();
        }

        if (!sneak_trigger->enabled)
            FUCK::BeginDisabled();

        if (FUCK::BeginTable("key", 2))
        {
            FUCK::TableNextColumn();
            auto key_scancode = static_cast<int>(sneak_trigger->key_scancode);
            if (FUCK::DragInt("Key (Scancode)", &key_scancode, 1.0f, 0, kGamepadOffset - 1))
                sneak_trigger->key_scancode = static_cast<uint32_t>(key_scancode);

            FUCK::TableNextColumn();
            if (FUCK::BeginTable("key name", 1, FUCK::TableFlags::kBorders))
            {
                FUCK::TableNextColumn();
                FUCK::TextUnformatted(scanCode2String(sneak_trigger->key_scancode).c_str());
                FUCK::EndTable();
            }
            FUCK::EndTable();
        }

        FUCK::Checkbox("Need Crouching", &sneak_trigger->need_crouch);
        FUCK::SameLine();
        FUCK::TextDisabled("[?]");
        if (FUCK::IsItemHovered())
            FUCK::SetTooltip("If disabled, you can trigger this while standing as long as you are not detected.\n"
                             "The animation will be different though.");

        if (!sneak_trigger->enabled)
            FUCK::EndDisabled();

        FUCK::PopID();
        FUCK::Unindent();
    }
}

void drawAnimationMenu()
{
    static std::string filter_text = {};
    static int         filter_mode = 0; // 0 None 1 ID 2 Tags

    auto  kaputt      = Kaputt::getSingleton();
    auto& tagexp_list = kaputt->tagexp_list;

    // Tag Expansions
    if (FUCK::BeginTable("tagexp config", 2))
    {
        FUCK::TableNextColumn();
        FUCK::AlignTextToFramePadding();
        FUCK::Text("Tag Expansion");
        FUCK::AlignTextToFramePadding();
        FUCK::SameLine();
        FUCK::TextDisabled("[?]");
        if (FUCK::IsItemHovered())
            FUCK::SetTooltip("If an animation has the tag on the left, then all tags on the right are provided.\n"
                             "Tags will be expanded only once i.e. the tags on the right cannot be expanded furthermore.");

        FUCK::TableNextColumn();
        if (FUCK::Button("Add"))
            tagexp_list.try_emplace("from", StrSet{"to"});

        FUCK::EndTable();
    }

    if (FUCK::BeginTable("tagexp", 3, FUCK::TableFlags::kBorders | FUCK::TableFlags::kScrollY | FUCK::TableFlags::kSizingStretchProp,
                         {0.f, (FUCK::GetFrameHeight()) * 5}))
    {
        FUCK::TableSetupColumn("from", FUCK::TableColumnFlags::kWidthStretch, 0.2f);
        FUCK::TableSetupColumn("arrow", FUCK::TableColumnFlags::kWidthStretch, 0.05f);
        FUCK::TableSetupColumn("to", FUCK::TableColumnFlags::kWidthStretch, 0.75f);

        std::string swap_from = {}, swap_to = {};
        for (auto& [from, to] : tagexp_list)
        {
            FUCK::PushID(from.c_str());

            FUCK::TableNextColumn();
            std::string temp_from = from;
            FUCK::SetNextItemWidth(-FLT_MIN);
            if (FUCK::InputText("##from", &temp_from, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                swap_from = from;
                swap_to   = temp_from;
            }
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Press Enter to apply. It will be sorted.\n"
                                 "If the tag already exists, nothing will happen.\n"
                                 "Leave this empty and press Enter to delete the item.");

            FUCK::TableNextColumn();
            FUCK::Text("->");

            FUCK::TableNextColumn();
            FUCK::SetNextItemWidth(-FLT_MIN);
            drawTagsInputText("##to", to);

            FUCK::PopID();
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

        FUCK::EndTable();
    }


    // anim filters
    if (FUCK::BeginTable("filtertab", 4, FUCK::TableFlags::kSizingStretchProp))
    {
        FUCK::TableSetupColumn("filter", FUCK::TableColumnFlags::kWidthStretch, 0.5f);
        FUCK::TableSetupColumn("1", FUCK::TableColumnFlags::kWidthStretch, 0.5f / 3);
        FUCK::TableSetupColumn("2", FUCK::TableColumnFlags::kWidthStretch, 0.5f / 3);
        FUCK::TableSetupColumn("3", FUCK::TableColumnFlags::kWidthStretch, 0.5f / 3);

        FUCK::TableNextColumn();
        FUCK::InputText("Filter by", &filter_text);

        FUCK::TableNextColumn();
        radioButton("None", &filter_mode, 0);
        FUCK::TableNextColumn();
        radioButton("ID", &filter_mode, 1);
        FUCK::TableNextColumn();
        radioButton("Tag", &filter_mode, 2);
        if (FUCK::IsItemHovered())
            FUCK::SetTooltip("Separate each tag with SPACE.");

        FUCK::EndTable();
    }

    // list of anims
    const auto table_flags =
        FUCK::TableFlags::kBorders | FUCK::TableFlags::kScrollY | FUCK::TableFlags::kSizingStretchProp;
    if (FUCK::BeginTable("Animation Entries", 2, table_flags, {0.f, -FLT_MIN}))
    {
        FUCK::TableSetupColumn("Editor ID", FUCK::TableColumnFlags::kWidthStretch, 0.4f);
        FUCK::TableSetupColumn("Tags", FUCK::TableColumnFlags::kWidthStretch, 0.6f);
        FUCK::TableHeadersRow();

        auto anim_list = kaputt->listAnims(filter_text, filter_mode);

        for (auto edid : anim_list)
        {

            FUCK::PushID(edid.data());

            FUCK::TableNextColumn();
            FUCK::AlignTextToFramePadding();
            if (kaputt->anim_custom_tags_map.contains(edid))
                FUCK::PushStyleColor(ImGuiCol_Text, {0.5f, 0.5f, 1.f, 1.f}); // indicate custom tags
            if (FUCK::Selectable(edid.data(), false))
                testPlayPairedIdle(RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid));
            if (kaputt->anim_custom_tags_map.contains(edid))
                FUCK::PopStyleColor();
            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Click to test it on the nearest NPC.\n"
                                 "Best when in a good position and they are not attacking.\n"
                                 "The conditions are not checked. So be wary.");

            FUCK::TableNextColumn();
            auto tags_str = joinTags(kaputt->getTags(edid));
            FUCK::SetNextItemWidth(-FLT_MIN);
            if (FUCK::InputText("##", &tags_str, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (tags_str.empty())
                    kaputt->anim_custom_tags_map.erase(edid);
                else
                    kaputt->setTags(edid, splitTags(tags_str));
            }

            if (FUCK::IsItemHovered())
                FUCK::SetTooltip("Press Enter to apply tag editing.\n"
                                 "The tags are sorted and seperated by SPACE.\n"
                                 "Leave empty and press Enter to set to default.\n"
                                 "(Remember to save to file afterwards.)");

            FUCK::PopID();
        }

        FUCK::EndTable();
    }
}

void drawMenu()
{
    auto kaputt = Kaputt::getSingleton();

    if (FUCK::BeginTable("fileops", 4))
    {
        FUCK::TableNextColumn();
        FUCK::AlignTextToFramePadding();
        FUCK::TextUnformatted("Config:");

        FUCK::TableNextColumn();
        FUCK::PushStyleColor(ImGuiCol_Button, {0.5f, 0.1f, 0.1f, 1.f});
        if (FUCK::Button("Save"))
            setStatusMessage(kaputt->saveConfig(def_config_path) ?
                                 std::format("Config saved to {}", def_config_path) :
                                 "Something went wrong while saving. Please check the log.");
        FUCK::PopStyleColor();

        FUCK::TableNextColumn();
        if (FUCK::Button("Save As Preset"))
            FUCK::OpenPopup("save config");
        if (FUCK::BeginPopup("save config"))
        {
            static std::string save_name = {};
            if (FUCK::InputText("Press Enter", &save_name, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                sanitizePresetName(save_name);
                setStatusMessage(
                    kaputt->saveConfig(config_dir + "\\"s + save_name + ".json") ?
                        "Config saved as " + save_name :
                        "Something went wrong while saving " + save_name + ". Please check the log.");
                FUCK::CloseCurrentPopup();
            }
            FUCK::EndPopup();
        }

        FUCK::TableNextColumn();
        if (FUCK::Button("Load Preset"))
            FUCK::OpenPopup("load config");
        if (FUCK::BeginPopup("load config"))
        {
            static std::string load_name  = {};
            bool               has_preset = false;

            if (fs::exists(config_dir))
                for (auto const& dir_entry : fs::directory_iterator{fs::path(config_dir)})
                    if (dir_entry.is_regular_file())
                        if (auto file_path = dir_entry.path(); file_path.extension() == ".json")
                        {
                            has_preset = true;
                            if (FUCK::Selectable(file_path.stem().string().c_str()))
                            {
                                load_name = file_path.stem().string();
                                setStatusMessage(
                                    kaputt->loadConfig(file_path.string()) ?
                                        "Loaded config preset " + load_name :
                                        "Something went wrong while loading " + load_name + ". Please check the log.");
                                FUCK::CloseCurrentPopup();
                            }
                        }

            if (!has_preset)
                FUCK::TextDisabled("No presets found.");

            FUCK::EndPopup();
        }

        FUCK::EndTable();
    }

    FUCK::Separator();

    FUCK::BeginChild("main", {0.f, -FUCK::GetTextLineHeightWithSpacing() - 2.f});
    if (FUCK::BeginTabBar("##"))
    {
        if (FUCK::BeginTabItem("Setting"))
        {
            drawSettingMenu();
            FUCK::EndTabItem();
        }
        if (FUCK::BeginTabItem("Trigger"))
        {
            drawTriggerMenu();
            FUCK::EndTabItem();
        }
        if (FUCK::BeginTabItem("Animation"))
        {
            drawAnimationMenu();
            FUCK::EndTabItem();
        }
        FUCK::EndTabBar();
    }
    FUCK::EndChild();

    std::string status;
    {
        std::scoped_lock lock(status_msg_mutex);
        status = status_msg;
    }
    FUCK::TextUnformatted(status.c_str());
}

class KaputtTool final : public FUCK::ITool
{
public:
    const char* Name() const override { return "Kaputt"; }

    void Draw() override { drawMenu(); }
};

void registerMenu()
{
    static KaputtTool tool;
    FUCK::RegisterTool(&tool);
}
} // namespace kaputt