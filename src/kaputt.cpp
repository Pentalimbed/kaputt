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
    all_ok &= anim_manager.loadAnims();
    all_ok &= loadConfig(def_config_path);

    ready.store(true);
    logger::info("Kaputt initialized.");

    return all_ok;
}

bool Kaputt::submit(
    RE::Actor*          attacker,
    RE::Actor*          victim,
    const TaggerOutput& extra_tags)
{
    auto anims      = anim_manager.listAnims();
    auto anims_size = anims.size();
    filter.filter(anims, attacker, victim, extra_tags);

    logger::debug("Filtered {} animations, {} remains.", anims_size, anims.size());
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
            preconds                     = j.at("preconds");
            filter                       = j.at("filter");
            anim_manager.custom_tags_map = j.at("custom_tags");
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

        ostream << json{
            {"preconds", preconds},
            {"filter", filter},
            {"custom_tags", anim_manager.custom_tags_map},
        };
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

} // namespace kaputt