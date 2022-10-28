#pragma once

namespace kaputt
{

class AnimManager
{
    friend class Kaputt;

private:
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
    StrMap<StrSet> tags_map        = {};
    StrMap<StrSet> custom_tags_map = {};

public:
    bool loadAnims();

    std::vector<std::string_view> listAnims(std::string_view filter_str = "", int filter_mode = 0);
    inline bool                   hasCustomTags(std::string_view edid) { return custom_tags_map.contains(edid); };
    const StrSet&                 getTags(std::string_view edid); // please make sure the tag is contained
    bool                          setTags(std::string_view edid, const StrSet& tags);
    inline void                   clearTags(std::string_view edid) { custom_tags_map.erase(edid); }
    inline void                   clearTags() { custom_tags_map.clear(); }
};

} // namespace kaputt
