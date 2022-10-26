#include "pipeline.h"

#include <random>

namespace kaputt
{
toml::table TaggerOutput::toToml()
{
    return toml::table{{"req_tags", required_tags.toToml()}, {"ban_tags", banned_tags.toToml()}};
}

TaggerOutput TaggerOutput::fromToml(toml::table tbl)
{
    if (!(tbl["req_tags"].as_array() && tbl["req_tags"].as_array()))
        throw std::runtime_error(R"(Required field ("req_tags" or "ban_tags") not fulfilled.)");

    TaggerOutput tags;
    tags.required_tags = StrSet::fromToml(*tbl["req_tags"].as_array());
    tags.banned_tags   = StrSet::fromToml(*tbl["ban_tags"].as_array());
    return tags;
}

toml::table Tagger::toToml()
{
    toml::table tbl{
        {"rule", rule->getName()},
        {"params", rule->params},
        {"comment", comment},
    };
    if (true_tags.has_value())
        tbl.emplace<toml::table>("true_tags", true_tags.value().toToml());
    if (false_tags.has_value())
        tbl.emplace<toml::table>("false_tags", false_tags.value().toToml());
}

Tagger Tagger::fromToml(toml::table tbl)
{
    if (!(tbl["rule"].as_string() && tbl["params"].as_table() && tbl["comment"].as_string()))
        throw std::runtime_error(R"(Required field ("rule" or "params" or "comment") not fulfilled.)");

    Tagger tagger;
    tagger.rule = Rule::getRule(tbl["rule"].ref<std::string>());

    if (!tagger.rule->checkParamsValidity(*tbl["params"].as_table()))
        throw std::runtime_error("Wrong parameters for rule type " + tbl["rule"].ref<std::string>() + '.');

    if (tbl.contains("true_tags"))
        tagger.true_tags = TaggerOutput::fromToml(*tbl["true_tags"].as_table());
    if (tbl.contains("false_tags"))
        tagger.false_tags = TaggerOutput::fromToml(*tbl["false_tags"].as_table());

    tagger.rule->init(*tbl["params"].as_table());
    tagger.comment = tbl["comment"].ref<std::string>();

    return tagger;
}

const AnimEntry* FilterPipeline::pickAnimation(const RE::Actor* attacker, const RE::Actor* victim) const
{
    auto tag_result = Tagger::tag(tagger_list, attacker, victim);

    std::vector<const AnimEntry*> filtered_anims;
    for (const auto& [edid, anim] : AnimEntryManager::getSingleton()->anim_dict)
    {
        StrSet        exp_tags  = {};
        const StrSet& orig_tags = anim.getTags();
        for (const auto& tag_exp : tag_exp_list)
            if (orig_tags.contains(tag_exp.from))
                exp_tags.merge(tag_exp.to);
        exp_tags.merge(orig_tags);

        if (std::ranges::all_of(tag_result.required_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }) &&
            std::ranges::none_of(tag_result.banned_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }))
            filtered_anims.push_back(&anim);
    }

    if (filtered_anims.empty())
        return nullptr;

    static std::default_random_engine     gen;
    std::uniform_int_distribution<double> unif(0.0, filtered_anims.size() - 1);
    std::once_flag                        flag;
    std::call_once(flag, [&]() { gen.seed(std::random_device()()); });

    return filtered_anims[unif(gen)];
}

} // namespace kaputt
