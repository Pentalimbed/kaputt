#pragma once

#include "kaputtAPI.h"

#include <nlohmann/json.hpp>

namespace kaputt
{

struct PreconditionParams
{
    enum class ESSENTIAL_PROT_ENUM : int
    {
        ENABLED,
        PROTECTED,
        DISABLED
    } essential_protection      = ESSENTIAL_PROT_ENUM::ENABLED;
    bool   protected_protection = true;
    float  last_hostile_range   = 1024;
    StrSet skipped_race         = {"FrostbiteSpiderRaceGiant",
                                   "SprigganMatronRace",
                                   "SprigganEarthMotherRace",
                                   "DLC2SprigganBurntRace",
                                   "DLC1LD_ForgemasterRace",
                                   "DLC2GhostFrostGiantRace"};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PreconditionParams, essential_protection, protected_protection, last_hostile_range, skipped_race)

struct TaggingRefs
{
    RE::TESIdleForm* idle_kaputt_root    = nullptr;
    RE::TESGlobal*   decap_requires_perk = nullptr;
    RE::TESGlobal*   decap_percent       = nullptr;
};

class Kaputt : public KaputtAPI
{
    friend void drawSettingMenu();
    friend void drawTriggerMenu();
    friend void drawAnimationMenu();

private:
    std::atomic_bool ready;

    /** Tags
     *  
     *  Delimited by SPACE
     * 
     *  -- Actor Weapon --
     *  e.g. a_dagger_l: attacker must wield dagger in left hand
     *  ONE HANDED: fist dagger sword axe mace staff + _l _r
     *  ALWAYS LEFT: shield torch
     *  TWO HANDED: sword2h axe2h mace2h bow crossbow
     *  COMMON: 1h_l 1h_r 2h all_l all_r all
     * 
     *  -- Actor Race --
     *  a_human
     *  human
     *  bear giant falmer hag cat spriggan centurion
     *  dragon troll wolf draugr chaurus(hunter) gargoyle
     *  boar riekling scrib lurker ballista vamplord werewolf
     * 
     *  -- Positioning --
     *  front back left right : relative to victim's orientation
     * 
     *  -- Misc --
     *  decap: decapitation
     *  adv: advancing
     *  sneak: sneaking killmove
     *  bleed: bleedout execution
     *  a_/v_player: player only
     */
    StrMap<StrSet> anim_tags_map        = {};
    StrMap<StrSet> anim_custom_tags_map = {};

    PreconditionParams precond_params = {};
    StrMap<StrSet>     tagexp_list    = {};

    TaggingRefs tagging_refs = {};

    bool loadTaggingParams();

public:
    // INIT
    static Kaputt* getSingleton()
    {
        static Kaputt kaputt;
        return std::addressof(kaputt);
    }
    bool                        init();
    virtual inline REL::Version getVersion() { return SKSE::PluginDeclaration::GetSingleton()->GetVersion(); }
    virtual inline bool         isReady() { return ready.load(); }

    // FILE IO
    bool loadAnims();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Kaputt, anim_custom_tags_map, precond_params, tagexp_list)
    bool loadConfig(std::string_view dir);
    bool saveConfig(std::string_view dir);

    // ANIM
    std::vector<std::string_view> listAnims(std::string_view filter_str = "", int filter_mode = 0);
    const StrSet&                 getTags(std::string_view edid); // please make sure the tag is in the map
    bool                          setTags(std::string_view edid, const StrSet& tags);

    // API
    virtual bool precondition(const RE::Actor* attacker, const RE::Actor* victim);
    virtual bool submit(RE::Actor*              attacker,
                        RE::Actor*              victim,
                        const SubmitInfoStruct& submit_info = {});
};
} // namespace kaputt