#pragma once

#include "utils.h"
#include "rules.h"

namespace kaputt
{
struct TaggerOutput
{
    StrSet required_tags = {};
    StrSet banned_tags   = {};

    void merge(const TaggerOutput& other)
    {
        mergeStrSet(required_tags, other.required_tags);
        mergeStrSet(banned_tags, other.banned_tags);
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

    inline TaggerOutput tag(RE::Actor* attacker, RE::Actor* victim) { return (*rule)(attacker, victim) ? true_tags.value_or(TaggerOutput{}) : false_tags.value_or(TaggerOutput{}); }

    toml::table   toToml();
    static Tagger fromToml(toml::table tbl);
};
} // namespace kaputt