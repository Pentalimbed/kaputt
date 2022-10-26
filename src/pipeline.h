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
    std::unique_ptr<Rule>       rule;
    std::string                 comment;
    std::optional<TaggerOutput> true_tags  = std::nullopt;
    std::optional<TaggerOutput> false_tags = std::nullopt;

    inline TaggerOutput tag(const RE::Actor* attacker, const RE::Actor* victim) const { return (*rule)(attacker, victim) ? true_tags.value_or(TaggerOutput{}) : false_tags.value_or(TaggerOutput{}); }

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