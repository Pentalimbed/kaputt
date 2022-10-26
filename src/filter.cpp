#include "filter.h"

#include <random>

namespace kaputt
{
toml::table TaggerOutput::toToml() const
{
    return toml::table{{"req_tags", required_tags.toToml()}, {"ban_tags", banned_tags.toToml()}};
}

TaggerOutput TaggerOutput::fromToml(const toml::table& tbl)
{
    if (!(tbl["req_tags"].as_array() && tbl["req_tags"].as_array()))
        throw std::runtime_error(R"(Required TaggerOutput field ("req_tags" or "ban_tags") unfulfilled.)");

    TaggerOutput tags;
    tags.required_tags = StrSet::fromToml(*tbl["req_tags"].as_array());
    tags.banned_tags   = StrSet::fromToml(*tbl["ban_tags"].as_array());
    return tags;
}

toml::table Tagger::toToml() const
{
    toml::table tbl{
        {"rule", rule->getName()},
        {"params", rule->params},
        {"comment", comment},
        {"enable_true", enable_true},
        {"enable_false", enable_false},
        {"true_tags", true_tags.toToml()},
        {"false_tags", false_tags.toToml()},
    };
}

Tagger Tagger::fromToml(const toml::table& tbl)
{
    if (!(tbl["rule"].as_string() && tbl["params"].as_table() && tbl["comment"].as_string() &&
          tbl["enable_true"].as_boolean() && tbl["enable_false"].as_boolean() &&
          tbl["true_tags"].as_table() && tbl["false_tags"].as_table()))
        throw std::runtime_error(R"(Required Tagger field(s) unfulfilled.)");

    Tagger tagger;
    tagger.rule = Rule::getRule(tbl["rule"].ref<std::string>());

    if (!tagger.rule->checkParamsValidity(*tbl["params"].as_table()))
        throw std::runtime_error("Wrong parameters for rule type " + tbl["rule"].ref<std::string>() + '.');

    tagger.true_tags  = TaggerOutput::fromToml(*tbl["true_tags"].as_table());
    tagger.false_tags = TaggerOutput::fromToml(*tbl["false_tags"].as_table());

    tagger.enable_true  = tbl["enable_true"].ref<bool>();
    tagger.enable_false = tbl["enable_false"].ref<bool>();
    tagger.rule->init(*tbl["params"].as_table());
    tagger.comment = tbl["comment"].ref<std::string>();

    return tagger;
}

TagExpansion TagExpansion::fromToml(const toml::table& tbl)
{
    if (!(tbl["from"].as_string() && tbl["to"].as_array()))
        throw std::runtime_error(R"(Required TagExpansion field(s) ("from" or "to") unfulfilled.)");

    TagExpansion tagexp;
    tagexp.to   = StrSet::fromToml(*tbl["to"].as_array());
    tagexp.from = tbl["from"].ref<std::string>();
    return tagexp;
}

const AnimEntry* FilterPipeline::pickAnimation(const RE::Actor* attacker, const RE::Actor* victim) const
{
    auto tag_result = Tagger::tag(tagger_list, attacker, victim);

    std::vector<const AnimEntry*> filtered_anims;
    for (const auto& [edid, anim] : AnimEntryManager::getSingleton()->anim_dict)
    {
        StrSet        exp_tags  = {};
        const StrSet& orig_tags = anim.getTags();
        for (const auto& tagexp : tagexp_list)
            if (orig_tags.contains(tagexp.from))
                exp_tags.merge(tagexp.to);
        exp_tags.merge(orig_tags);

        if (std::ranges::all_of(tag_result.required_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }) &&
            std::ranges::none_of(tag_result.banned_tags, [&](const std::string& tag) { return exp_tags.contains(tag); }))
            filtered_anims.push_back(&anim);
    }

    if (filtered_anims.empty())
        return nullptr;

    static std::default_random_engine     gen;
    std::uniform_int_distribution<size_t> unif(0, filtered_anims.size() - 1);
    std::once_flag                        flag;
    std::call_once(flag, [&]() { gen.seed(std::random_device()()); });

    return filtered_anims[unif(gen)];
}

void FilterPipeline::loadFile(fs::path dir, bool append)
{
    if (!append)
        clear();

    auto result = toml::parse_file(dir.c_str());
    if (!result)
    {
        auto err = result.error();
        logger::warn("Failed to parse filter file {}. Error: {} (Line {}, Col {}).",
                     dir.string(), err.description(), err.source().begin.line, err.source().begin.column);
        return;
    }
    logger::info("Parsing filter file {}", dir.string());

    auto tbl = result.table();
    if (auto taggers_arr = tbl["taggers"].as_array())
        for (auto& v : *taggers_arr)
        {
            if (auto tagger_tbl = v.as_table())
            {
                try
                {
                    tagger_list.push_back(Tagger::fromToml(*tagger_tbl));
                }
                catch (std::runtime_error e)
                {
                    logger::warn("Failed to parse one of the taggers. Error: {}", e.what());
                }
            }
            else
                logger::warn("Failed to parse one of the taggers. Error: Wrong data type.");
        }
    else
        logger::warn(R"(Required "taggers" field unfulfilled. Skipped.)");

    if (auto taggers_arr = tbl["tagexps"].as_array())
        for (auto& v : *taggers_arr)
        {
            if (auto tagger_tbl = v.as_table())
            {
                try
                {
                    tagger_list.push_back(Tagger::fromToml(*tagger_tbl));
                }
                catch (std::runtime_error e)
                {
                    logger::warn("Failed to parse one of the tag expansions. Error: {}", e.what());
                }
            }
            else
                logger::warn("Failed to parse one of the tag expansions. Error: Wrong data type.");
        }
    else
        logger::warn(R"(Required "tagexps" field unfulfilled. Skipped.)");
}

void FilterPipeline::saveFile(fs::path dir) const
{
    logger::info("Saving filter file {}.", dir.string());

    std::ofstream f(dir);
    if (!f.is_open())
    {
        logger::error("Failed to write at {}!", dir.string());
        return;
    }

    toml::array taggers_arr = {};
    for (const auto& tagger : tagger_list)
        taggers_arr.emplace_back<toml::table>(tagger.toToml());
    toml::array tagexp_arr = {};
    for (const auto& tagexp : tagger_list)
        tagexp_arr.emplace_back<toml::table>(tagexp.toToml());

    f << toml::table{{"taggers", taggers_arr}, {"tagexps", tagexp_arr}};
}

} // namespace kaputt
