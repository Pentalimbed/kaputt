#include "animation.h"
#include "re.h"
#include "menu.h"

#include <sstream>

StrSet tomlArray2Tags(const toml::array* arr)
{
    StrSet tags;
    for (const auto& tag : *arr)
        if (tag.is_string())
            tags.insert(tag.ref<std::string>());
    return tags;
}

namespace kaputt
{
void AnimEntry::parse_toml_array(const toml::array& arr, bool is_custom)
{
    if (is_custom)
        custom_tags.emplace(tomlArray2StrSet(arr));
    else
        tags = tomlArray2StrSet(arr);
}

void AnimEntry::play(RE::Actor* attacker, RE::Actor* victim)
{
    logger::debug("Now playing {}", editor_id);
    playPairedIdle(attacker->GetActorRuntimeData().currentProcess, attacker, RE::DEFAULT_OBJECT::kActionIdle, idle_form, true, false, victim);
    setStatusMessage("Last played by this mod: " + editor_id); // notify menu
}

void AnimEntry::testPlay(float max_range)
{
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player)
    {
        logger::info("No player found!");
        return;
    }
    auto victim = getNearestActor(player, max_range);
    if (!victim)
    {
        logger::info("No target found!");
        return;
    }

    play(player, victim);
}

void AnimEntryManager::loadSingleEntryFile(fs::path dir)
{
    auto result = toml::parse_file(dir.c_str());
    if (!result)
    {
        auto err = result.error();
        logger::warn("Failed to parse file {}. Error: {} (Line {}, Col {})",
                     dir.string(), err.description(), err.source().begin.line, err.source().begin.column);
        return;
    }
    logger::info("Parsing file {}", dir.string());

    auto     tbl        = result.table();
    uint16_t anim_count = 0;
    for (auto [edid, v] : tbl)
        if (v.is_array())
        {
            auto form = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid);
            if (!form)
            {
                logger::warn("Failed to parse entry {}. Error: Cannot find an IdleForm with this EditorId!", edid);
                continue;
            }
            if (anim_dict.contains(edid.str()))
            {
                logger::warn("Failed to parse entry {}. Error: An entry with the same EditorId has been registered!", edid);
                continue;
            }

            AnimEntry entry{
                .editor_id = std::string(edid),
                .idle_form = form,
            };
            entry.parse_toml_array(*v.as_array());
            anim_dict[std::string(edid)] = std::move(entry);
            logger::info("Added entry {}.", edid);
            ++anim_count;
        }
    logger::info("Parsed file {}. {} animations were registered.", dir.string(), anim_count);
}

void AnimEntryManager::loadAllEntryFiles()
{
    logger::info("Reading animations...");

    auto full_anim_dir = fs::path(plugin_dir) / fs::path(config_dir) / fs::path(anim_dir);
    for (auto const& dir_entry : fs::directory_iterator{full_anim_dir})
        if (dir_entry.is_regular_file())
            if (auto file_path = dir_entry.path(); file_path.extension() == ".toml")
                loadSingleEntryFile(file_path);
}

void AnimEntryManager::loadCustomFile(fs::path dir)
{
    clearCustomTags();

    auto result = toml::parse_file(dir.c_str());
    if (!result)
    {
        auto err = result.error();
        logger::warn("Failed to parse file {}. Error: {} (Line {}, Col {}).",
                     dir.string(), err.description(), err.source().begin.line, err.source().begin.column);
        return;
    }
    logger::info("Parsing customization file {}", dir.string());

    auto tbl = result.table();
    for (auto [edid, v] : tbl)
        if (v.is_array())
        {
            if (!anim_dict.contains(edid.str()))
            {
                logger::info("Customization entry {} is not registered. This is ok.", edid);
                continue;
            }

            anim_dict[std::string(edid)].parse_toml_array(*v.as_array(), true);
            logger::info("Added customization entry {}", edid);
        }
    logger::info("Parsed customization file {}.", dir.string());
}

void AnimEntryManager::saveCustomFile(fs::path dir)
{
    logger::info("Saving customization file {}.", dir.string());

    std::ofstream f(dir);
    if (!f.is_open())
    {
        logger::error("Failed to write at {}!", dir.string());
        return;
    }

    toml::table tbl = {};
    for (const auto& [edid, anim] : anim_dict)
        if (anim.custom_tags.has_value())
            tbl.emplace<toml::array>(edid, strSet2TomlArray(anim.custom_tags.value()));
    f << tbl;
    logger::info("Saved customization file {}.", dir.string());
}

void AnimEntryManager::clearCustomTags()
{
    for (auto& [edid, anim] : anim_dict)
        anim.custom_tags = std::nullopt;
    logger::info("All custom tags cleared.");
}
} // namespace kaputt