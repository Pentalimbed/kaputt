#include "kaputt.h"

#include "re.h"
#include "utils.h"
#include "trigger.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <effolkronium/random.hpp>

namespace kaputt
{
bool Kaputt::loadRefs()
{
    required_refs.vanilla_killmove        = RE::TESForm::LookupByEditorID<RE::TESGlobal>("Killmove");
    required_refs.idle_kaputt_root        = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("KaputtRoot");
    required_refs.decap_requires_perk     = RE::TESForm::LookupByEditorID<RE::TESGlobal>("KapReqDecapPerk");
    required_refs.decap_bleed_ignore_perk = RE::TESForm::LookupByEditorID<RE::TESGlobal>("KapBleedIgnoreDecapPerk");
    required_refs.decap_percent           = RE::TESForm::LookupByEditorID<RE::TESGlobal>("KapDecapPercent");
    required_refs.decap_use_chance        = RE::TESForm::LookupByEditorID<RE::TESGlobal>("KapDecapUseChance");
    return required_refs.vanilla_killmove &&
        required_refs.idle_kaputt_root &&
        required_refs.decap_requires_perk &&
        required_refs.decap_bleed_ignore_perk &&
        required_refs.decap_use_chance &&
        required_refs.decap_percent;
}

void Kaputt::applyRefs()
{
    required_refs.vanilla_killmove->value        = !misc_params.disable_vanilla;
    required_refs.decap_requires_perk->value     = tagging_params.decap_requires_perk;
    required_refs.decap_bleed_ignore_perk->value = tagging_params.decap_bleed_ignore_perk;
    required_refs.decap_percent->value           = tagging_params.decap_percent;
    required_refs.decap_use_chance->value        = tagging_params.decap_use_chance;
}

bool Kaputt::init()
{
    logger::info("Initializing all Kaputt components.");

    bool all_ok = true;

    if (!loadRefs())
    {
        logger::error("Cannot find 'KaputtRoot', 'KapReqDecapPerk' or 'KapDecapPercent', mod disabled. Make sure KaputtVanillaKillmoves.esp is enabled in your load order.");
        return false;
    }

    all_ok &= loadAnims();
    all_ok &= loadConfig(def_config_path);

    ready.store(true);
    logger::info("Kaputt initialized.");

    return all_ok;
}

std::vector<std::string_view> Kaputt::listAnims(std::string_view filter_str, int filter_mode)
{
    std::vector<std::string_view> retval;
    for (auto const& [edid, _] : anim_tags_map)
    {
        if ((filter_mode == 1) && !edid.contains(filter_str))
            continue;
        if ((filter_mode == 2) && !std::ranges::all_of(splitTags(filter_str), [&](const std::string& tag) { return getTags(edid).contains(tag); }))
            continue;
        retval.push_back(edid);
    }
    return retval;
}

const StrSet& Kaputt::getTags(std::string_view edid)
{
    auto result_tags        = anim_tags_map.find(edid);
    auto result_custom_tags = anim_custom_tags_map.find(edid);
    return (result_custom_tags == anim_custom_tags_map.end()) ? result_tags->second : result_custom_tags->second;
}

bool Kaputt::setTags(std::string_view edid, const StrSet& tags)
{
    if (auto result_tags = anim_tags_map.find(edid); result_tags != anim_tags_map.end())
    {
        anim_custom_tags_map.insert_or_assign(std::string{edid}, tags);
        return true;
    }
    else
        return false;
}

bool Kaputt::loadAnims()
{
    logger::info("Loading animation entries...");

    bool all_ok = true;

    if (!fs::exists(anim_dir))
    {
        logger::warn("Animation folder doesn't exist.");
        return false;
    }

    for (auto const& dir_entry : fs::directory_iterator{anim_dir})
        if (dir_entry.is_regular_file())
            if (auto file_path = dir_entry.path(); file_path.extension() == ".json")
            {
                logger::info("Reading {}", file_path.string());

                std::ifstream istream{file_path};
                if (!istream.is_open())
                {
                    logger::warn("Failed to open {}", file_path.filename().string());
                    all_ok = false;
                    continue;
                }

                json j;
                try
                {
                    j = json::parse(istream);
                }
                catch (json::parse_error& e)
                {
                    logParseError(e);
                    all_ok = false;
                    continue;
                }

                StrMap<StrSet> new_tags = {};
                try
                {
                    new_tags = j;
                }
                catch (json::exception& e)
                {
                    logJsonException("StrMap<StrSet>", e);
                    all_ok = false;
                    continue;
                }
                std::erase_if(new_tags, [&](const auto& item) {
                    auto const& [edid, tags] = item;
                    auto form                = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid);
                    if (!form)
                    {
                        logger::warn("Cannot find IdleForm {}!", edid);
                        all_ok = false;
                        return true;
                    }
                    return false;
                });

                auto anim_count = new_tags.size();
                anim_tags_map.merge(new_tags);

                logger::info("Successfully registered {} animations in {}", anim_count, file_path.filename().string());
            }

    logger::info("All animation entries loaded. Total animation count: {}", anim_tags_map.size());
    return all_ok;
}

bool Kaputt::loadConfig(std::string_view dir)
{
    logger::info("Loading kaputt config {} ...", dir);

    bool all_ok = true;
    if (fs::path file_path = dir; file_path.extension() == ".json")
    {
        logger::info("Reading {}", file_path.string());

        std::ifstream istream{file_path};
        if (!istream.is_open())
        {
            logger::warn("Failed to open {}", file_path.filename().string());
            logger::warn("Kaputt config not loaded!");
            return false;
        }

        json j;
        try
        {
            j = json::parse(istream);
        }
        catch (json::parse_error& e)
        {
            logParseError(e);
            logger::warn("Kaputt config not loaded!");
            return false;
        }

        try
        {
            from_json(j, *this);
            from_json(j["triggers"]["vanilla"], *VanillaTrigger::getSingleton());
            from_json(j["triggers"]["post_hit"], *PostHitTrigger::getSingleton());
            from_json(j["triggers"]["sneak"], *SneakTrigger::getSingleton());
        }
        catch (json::exception e)
        {
            logJsonException("Kaputt", e);
            logger::warn("Kaputt config not fully loaded!");
            return false;
        }
    }
    else
    {
        logger::warn("{} is not a json file!", dir);
        logger::warn("Kaputt config not loaded!");
        return false;
    }

    logger::info("Kaputt config loaded.");
    return all_ok;
}
bool Kaputt::saveConfig(std::string_view dir)
{
    logger::info("Saving kaputt config {} ...", dir);

    bool all_ok = true;
    if (fs::path file_path = dir; file_path.extension() == ".json")
    {
        logger::info("Reading {}", file_path.string());

        std::ofstream ostream{file_path};
        if (!ostream.is_open())
        {
            logger::warn("Failed to open {}", file_path.filename().string());
            logger::warn("Kaputt config not saved!");
            return false;
        }

        json j = *this;
        j.emplace("triggers", json{});
        j["triggers"].emplace("vanilla", *VanillaTrigger::getSingleton());
        j["triggers"].emplace("post_hit", *PostHitTrigger::getSingleton());
        j["triggers"].emplace("sneak", *SneakTrigger::getSingleton());
        ostream << j.dump(4);
    }
    else
    {
        logger::warn("{} is not a json file!", dir);
        logger::warn("Kaputt config not saved!");
        return false;
    }

    logger::info("Kaputt config saved.");
    return all_ok;
}

bool Kaputt::precondition(const RE::Actor* attacker, const RE::Actor* victim)
{
    logger::debug("precondition");
    // Playable check
    if (!(animPlayable(attacker) && animPlayable(victim)))
        return false;
    // Essential check
    if (victim->IsEssential())
        switch (precond_params.essential_protection)
        {
            case PreconditionParams::ESSENTIAL_PROT_ENUM::ENABLED:
                return false;
            case PreconditionParams::ESSENTIAL_PROT_ENUM::PROTECTED:
                if (!attacker->IsPlayerRef())
                    return false;
                break;
            default:
                break;
        }
    // Protected check
    if (precond_params.protected_protection && victim->IsProtected() && !attacker->IsPlayerRef())
        return false;
    // Furniture anim check
    if (isFurnitureAnimType(victim, RE::BSFurnitureMarker::AnimationType::kSit) && !precond_params.furn_sit)
        return false;
    if (isFurnitureAnimType(victim, RE::BSFurnitureMarker::AnimationType::kLean) && !precond_params.furn_lean)
        return false;
    if (isFurnitureAnimType(victim, RE::BSFurnitureMarker::AnimationType::kSleep) && !precond_params.furn_sleep)
        return false;
    // Height diff check
    if (auto height_diff = victim->GetPositionZ() - attacker->GetPositionZ();
        (height_diff < precond_params.height_diff_range[0]) || (height_diff > precond_params.height_diff_range[1]))
        return false;
    // Last hostile check
    if (!isLastHostileInRange(attacker, victim, precond_params.last_hostile_range))
        return false;
    // Race filters
    if (std::ranges::any_of(std::array{attacker, victim}, [&](auto actor) { return precond_params.skipped_race.contains(actor->GetRace()->GetFormEditorID()); }))
        return false;
    // Finally
    return true;
}

bool Kaputt::submit(RE::Actor* attacker, RE::Actor* victim, const SubmitInfoStruct& submit_info)
{
    logger::debug("filtering");

    std::vector<std::string_view> anims        = listAnims();
    StrMap<StrSet>                exp_tags_map = {};
    for (auto& edid : anims)
    {
        StrSet exp_tags  = {};
        auto&  orig_tags = getTags(edid);

        for (const auto& [from, to] : tagexp_list)
            if (orig_tags.contains(from))
                mergeSet(exp_tags, to);
        mergeSet(exp_tags, orig_tags);

        exp_tags_map.emplace(edid, std::move(exp_tags));
    }

    // manual req and ban
    std::erase_if(anims, [&](auto& edid) {
        return std::ranges::any_of(tagging_params.required_tags, [&](auto& tag) { return !exp_tags_map.find(edid)->second.contains(tag); }) ||
            std::ranges::any_of(submit_info.required_tags, [&](auto& tag) { return !exp_tags_map.find(edid)->second.contains(tag); }) ||
            std::ranges::any_of(tagging_params.banned_tags, [&](auto& tag) { return exp_tags_map.find(edid)->second.contains(tag); }) ||
            std::ranges::any_of(submit_info.banned_tags, [&](auto& tag) { return exp_tags_map.find(edid)->second.contains(tag); });
    });

    // skeleton tag
    auto att_race_tag = "a_" + getSkeletonRace(attacker);
    auto vic_race_tag = "v_" + getSkeletonRace(victim);
    std::erase_if(anims, [&](auto& edid) {
        return !(
            (att_race_tag == "a_" || exp_tags_map.find(edid)->second.contains(att_race_tag)) &&
            (vic_race_tag == "v_" || exp_tags_map.find(edid)->second.contains(vic_race_tag)));
    });

    // IdleTaggerLOL
    if (required_refs.idle_kaputt_root->childIdles)
    {
        StrMap<bool>             item_results = {};
        RE::ConditionCheckParams params(attacker->As<RE::TESObjectREFR>(), victim->As<RE::TESObjectREFR>());
        for (auto form : *required_refs.idle_kaputt_root->childIdles)
        {
            if (anims.empty())
                break;

            auto             idle_form = form->As<RE::TESIdleForm>();
            std::string_view idle_edid = idle_form->GetFormEditorID();
            auto&            flags     = idle_form->data.flags;

            bool result = true;
            if (flags.all(RE::IDLE_DATA::Flag::kSequence)) // check each individually
            {
                bool or_cache = false;
                for (auto cond_item = idle_form->conditions.head; cond_item != nullptr; cond_item = cond_item->next)
                {
                    auto& cond_data = cond_item->data;

                    bool single_result;
                    if (cond_data.flags.swapTarget && (cond_data.functionData.function == RE::FUNCTION_DATA::FunctionID::kGetGraphVariableInt)) // reference checked item
                    {
                        std::string_view ref_item = static_cast<RE::BSString*>(cond_data.functionData.params[0])->c_str();
                        if (item_results.contains(ref_item))
                        {
                            single_result = item_results.find(ref_item)->second;
                            single_result = single_result == (bool)(cond_data.comparisonValue.f);
                            single_result = single_result == (cond_data.flags.opCode == RE::CONDITION_ITEM_DATA::OpCode::kEqualTo);
                        }
                        else
                        {
                            single_result = false;
                            logger::warn("One condition from {} requires an unknown item {}.", idle_edid, ref_item);
                        }
                    }
                    else
                        single_result = cond_item->IsTrue(params);

                    or_cache |= single_result;
                    if (!cond_item->next || !cond_data.flags.isOR)
                    {
                        result &= or_cache;
                        or_cache = false;
                    }
                }
            }
            else
                result = idle_form->conditions(attacker->As<RE::TESObjectREFR>(), victim->As<RE::TESObjectREFR>());

            if (auto tag_idx = idle_edid.find_first_of('_'); tag_idx != std::string::npos) // Has tag
            {
                std::string_view tag{&idle_edid[tag_idx + 1]};
                std::string_view req_tag = result ? tag : std::string_view{};
                std::string_view ban_tag = (!result && flags.all(RE::IDLE_DATA::Flag::kNoAttacking)) ? tag : std::string_view{};

                if (!(req_tag.empty() && ban_tag.empty()))
                {
                    if (flags.all(RE::IDLE_DATA::Flag::kBlocking))
                        std::swap(req_tag, ban_tag);

                    // if ((req_tag == "decap") &&
                    //     std::ranges::none_of(anims, [&](auto edid) { return exp_tags_map.find(edid)->second.contains("decap"); })) // special treatment for decap
                    //     continue;

                    std::erase_if(anims, [&](auto edid) {
                        return (!req_tag.empty() && !exp_tags_map.find(edid)->second.contains(req_tag)) ||
                            (!ban_tag.empty() && exp_tags_map.find(edid)->second.contains(ban_tag));
                    });
                }
            }

            logger::debug("Tagger item {}, result {}, {} left", idle_edid, result, anims.size());

            item_results.emplace(idle_form->GetFormEditorID(), result);
        }
    }

    logger::debug("Filter over, {} of {} left", anims.size(), anim_tags_map.size());
    if (anims.empty())
        return false;

    auto edid = anims[effolkronium::random_static::get(0ull, anims.size() - 1)];
    if (auto idle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid); idle)
    {
        // preprocess
        attacker->NotifyAnimationGraph("attackStop");
        victim->NotifyAnimationGraph("attackStop");
        attacker->NotifyAnimationGraph("staggerStop");
        victim->NotifyAnimationGraph("staggerStop");
        if ((victim->AsActorState()->GetKnockState() == RE::KNOCK_STATE_ENUM::kGetUp) ||
            (victim->AsActorState()->GetKnockState() == RE::KNOCK_STATE_ENUM::kQueued))
        {
            victim->AsActorState()->actorState1.knockState = RE::KNOCK_STATE_ENUM::kNormal;
            victim->NotifyAnimationGraph("GetUpEnd");
        }

        playPairedIdle(idle, attacker, victim);
        return true;
    }
    else
    {
        logger::warn("Registered animation {} has no corresponding IdleForm. Please report to the author.", edid);
        return false;
    }
}

} // namespace kaputt