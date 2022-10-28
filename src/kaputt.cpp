#include "kaputt.h"

#include "re.h"
#include "utils.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <effolkronium/random.hpp>

namespace kaputt
{

bool Kaputt::init()
{
    logger::info("Initializing all Kaputt components.");

    bool all_ok = true;
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

        ostream << json{*this};
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

bool Kaputt::submit(RE::Actor* attacker, RE::Actor* victim, const TaggerOutput& extra_tags)
{
    std::vector<std::string_view> anims = {};

    auto tag_result = Tagger::tag(tagger_list, attacker, victim);
    tag_result.merge(extra_tags);

    std::erase_if(anims, [&](std::string_view edid) {
        StrSet exp_tags  = {};
        auto&  orig_tags = getTags(edid);

        for (const auto& [from, to] : tagexp_list)
            if (orig_tags.contains(from))
                exp_tags.merge(const_cast<StrSet&>(to));
        exp_tags.merge(const_cast<StrSet&>(orig_tags));

        return !(std::ranges::all_of(tag_result.required_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }) &&
                 std::ranges::none_of(tag_result.banned_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }));
    });

    logger::debug("Filtered {} animations, {} remains.", anim_tags_map.size(), anims.size());
    if (anims.empty())
        return false;

    auto edid = anims[effolkronium::random_static::get(0ull, anims.size() - 1)];
    if (auto idle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid); idle)
    {
        playPairedIdle(idle, attacker, victim); // TODO: clear
        return true;
    }
    else
    {
        logger::warn("Registered animation {} has no corresponding IdleForm. Please report to the author.", edid);
        return false;
    }
}

} // namespace kaputt