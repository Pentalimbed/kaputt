#pragma once

#include "utils.h"
#include "rules.h"
#include "animation.h"

namespace kaputt
{
struct TaggerOutput
{
    StrSet required_tags = {};
    StrSet banned_tags   = {};

    void merge(const TaggerOutput& other)
    {
        required_tags.merge(other.required_tags);
        banned_tags.merge(other.banned_tags);
    }

    toml::table         toToml();
    static TaggerOutput fromToml(toml::table tbl);
};

struct Tagger
{
    std::unique_ptr<Rule> rule;
    std::string           comment;
    bool                  enable_true  = false;
    bool                  enable_false = false;
    TaggerOutput          true_tags    = {};
    TaggerOutput          false_tags   = {};

    inline TaggerOutput tag(const RE::Actor* attacker, const RE::Actor* victim) const
    {
        if (enable_true || enable_false) // optimization
            return (*rule)(attacker, victim) ?
                (enable_true ? true_tags : TaggerOutput{}) :
                (enable_false ? false_tags : TaggerOutput{});
        else
            return {};
    }

    toml::table   toToml();
    static Tagger fromToml(toml::table tbl);

    inline static TaggerOutput tag(const std::vector<Tagger>& tagger_list, const RE::Actor* attacker, const RE::Actor* victim)
    {
        TaggerOutput output = {};
        for (const auto& tagger : tagger_list)
            output.merge(tagger.tag(attacker, victim));
        return output;
    }
};

struct TagExpansion
{
    std::string from;
    StrSet      to;
};

class FilterPipeline
{
private:
    std::vector<Tagger>       tagger_list;
    std::vector<TagExpansion> tag_exp_list;

public:
    static FilterPipeline* getSingleton()
    {
        static FilterPipeline pipe;
        return std::addressof(pipe);
    }

    const AnimEntry* pickAnimation(const RE::Actor* attacker, const RE::Actor* victim) const;
};

} // namespace kaputt