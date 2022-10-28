#pragma once

#include "rule.h"

namespace kaputt
{

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

struct TaggerOutput
{
    StrSet required_tags = {};
    StrSet banned_tags   = {};

    void merge(const TaggerOutput& other);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TaggerOutput, required_tags, banned_tags)

struct Tagger
{
    RuleInfo     rule         = {};
    bool         enable_true  = false;
    bool         enable_false = false;
    TaggerOutput true_tags    = {};
    TaggerOutput false_tags   = {};

    inline TaggerOutput tag(const RE::Actor* attacker, const RE::Actor* victim) const
    {
        if (enable_true || enable_false) // optimization
            return rule.check(attacker, victim) ?
                (enable_true ? true_tags : TaggerOutput{}) :
                (enable_false ? false_tags : TaggerOutput{});
        else
            return {};
    }
    inline static TaggerOutput tag(const std::vector<Tagger>& tagger_list, const RE::Actor* attacker, const RE::Actor* victim)
    {
        TaggerOutput output = {};
        for (const auto& tagger : tagger_list)
            output.merge(tagger.tag(attacker, victim));
        return output;
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tagger, rule, enable_true, enable_false, true_tags, false_tags)

class Kaputt
{
    friend void drawPreconditionMenu();
    friend void drawTriggerMenu();
    friend void drawFilterMenu();
    friend void drawAnimationMenu();

private:
    std::atomic_bool ready;

    StrMap<StrSet> anim_tags_map = {};

    std::vector<RuleInfo> preconds = {};

    std::vector<Tagger> tagger_list          = {};
    StrMap<StrSet>      tagexp_list          = {};
    StrMap<StrSet>      anim_custom_tags_map = {};

public:
    // INIT
    static Kaputt* getSingleton()
    {
        static Kaputt kaputt;
        return std::addressof(kaputt);
    }
    bool                init();
    virtual inline bool isReady() { return ready.load(); }

    // FILE IO
    bool loadAnims();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Kaputt, preconds, tagger_list, tagexp_list, anim_custom_tags_map)
    bool loadConfig(std::string_view dir);
    bool saveConfig(std::string_view dir);

    // ANIM
    std::vector<std::string_view> listAnims(std::string_view filter_str = "", int filter_mode = 0);
    const StrSet&                 getTags(std::string_view edid); // please make sure the tag is in the map
    bool                          setTags(std::string_view edid, const StrSet& tags);

    // API
    inline bool precondition(const RE::Actor* attacker, const RE::Actor* victim)
    {
        return std::ranges::all_of(preconds, [=](RuleInfo& rule) { return (!rule.enabled) || (rule.need_true == rule.check(attacker, victim)); });
    }
    bool submit(RE::Actor* attacker, RE::Actor* victim, const TaggerOutput& extra_tags = {});
};
} // namespace kaputt