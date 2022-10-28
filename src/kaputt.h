#pragma once

#include "rule.h"

namespace kaputt
{

struct TaggerOutput
{
    StrSet required_tags = {};
    StrSet banned_tags   = {};

    void merge(const TaggerOutput& other)
    {
        required_tags.merge(const_cast<StrSet&>(other.required_tags)); // WHY IS MERGE NOT CONST I HAVE NO IDEA
        banned_tags.merge(const_cast<StrSet&>(other.banned_tags));
    }
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
            return getRule().at(rule.type)->check(rule.params, attacker, victim) ?
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
    friend void drawFilterMenu();
    friend void drawAnimationMenu();

private:
    std::atomic_bool ready;

    std::vector<RuleInfo> preconds = {};

    std::vector<Tagger> tagger_list;
    StrMap<StrSet>      tagexp_list;
    StrMap<StrSet>      anim_tags_map        = {};
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
        return std::ranges::all_of(preconds, [=](RuleInfo& rule) { return rule.enabled == rule.check(attacker, victim); });
    }
    bool submit(RE::Actor* attacker, RE::Actor* victim, const TaggerOutput& extra_tags = {});
};
} // namespace kaputt