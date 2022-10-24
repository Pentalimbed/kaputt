#pragma once

#include "utils.h"

#include <filesystem>
#include <optional>

#include <toml++/toml.h>

namespace kaputt
{
namespace fs = std::filesystem;


struct AnimEntry
{
    std::string      editor_id;
    RE::TESIdleForm* idle_form;

    /** Tags
     *  
     *  Delimited by SPACE
     * 
     *  -- Actor Weapon --
     *  e.g. a_dagger_l: attacker must wield dagger in left hand
     *  ONE HANDED: fist dagger sword axe mace + _l _r
     *  ALWAYS LEFT: staff shield torch
     *  TWO HANDED: greatsword blunt2h bow crossbow
     *  COMMON: 1h_l 1h_r 2h all_l all_r all
     * 
     *  -- Actor Race --
     *  a_r_human
     *  human
     *  bear giant
     * 
     *  -- Positioning --
     *  front back left right : relative to victim's orientation
     * 
     *  -- Misc --
     *  decap: decapitation
     *  adv: advancing
     *  sneak: sneaking killmove
     *  bleed: bleedout execution
     */
    StrSet                tags;
    std::optional<StrSet> custom_tags = std::nullopt;
    inline StrSet&        getTags() { return custom_tags.has_value() ? custom_tags.value() : tags; }

    void parse_toml_array(const toml::array* arr, bool is_custom = false) throw(std::runtime_error);
};

class AnimEntryManager
{
public:
    static AnimEntryManager* getSingleton()
    {
        static AnimEntryManager manager;
        return std::addressof(manager);
    }

    void            loadAllEntryFiles();
    void            loadSingleEntryFile(fs::path dir);
    inline fs::path getDefaultCustomFilePath() { return fs::path(plugin_dir) / fs::path(config_dir) / fs::path(anim_dir) / fs::path(anim_custom_dir) / fs::path(anim_custom_def); }
    void            loadCustomFile(fs::path dir);
    void            saveCustomFile(fs::path dir);
    void            clearCustomTags();

    StrMap<AnimEntry> anim_dict;
};
} // namespace kaputt
