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

class FilterPipeline
{
    friend void drawFilterMenu();

private:
    std::vector<Tagger> tagger_list;
    StrMap<StrSet>      tagexp_list;

public:
    inline void clear()
    {
        tagger_list.clear();
        tagexp_list.clear();
    }

    void filter(std::vector<std::string_view>& anims, const RE::Actor* attacker, const RE::Actor* victim, const TaggerOutput& extra_tags = {}) const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(FilterPipeline, tagger_list, tagexp_list)
};

} // namespace kaputt