#include "tagger.h"

namespace kaputt
{
toml::table TaggerOutput::toToml()
{
    return toml::table{{"req_tags", strSet2TomlArray(required_tags)}, {"ban_tags", strSet2TomlArray(banned_tags)}};
}

TaggerOutput TaggerOutput::fromToml(toml::table tbl)
{
    if (!(tbl["req_tags"].as_array() && tbl["req_tags"].as_array()))
        throw std::runtime_error(R"(Required field ("req_tags" or "ban_tags") not fulfilled.)");

    TaggerOutput tags;
    tags.required_tags = tomlArray2StrSet(*tbl["req_tags"].as_array());
    tags.banned_tags   = tomlArray2StrSet(*tbl["ban_tags"].as_array());
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
} // namespace kaputt
