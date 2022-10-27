#include "animation.h"

#include "utils.h"
#include "re.h"
#include "menu.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace kaputt
{
std::vector<std::string_view> AnimManager::listAnims(std::string_view filter_str, int filter_mode)
{
    std::vector<std::string_view> retval;
    for (auto const& [edid, _] : tags_map)
    {
        if ((filter_mode == 1) && !edid.contains(filter_str))
            continue;
        if ((filter_mode == 2) && !std::ranges::all_of(splitTags(filter_str), [&](const std::string& tag) { return getTags(edid)->contains(tag); }))
            continue;
        retval.push_back(edid);
    }
    return retval;
}

const StrSet* AnimManager::getTags(std::string_view edid)
{
    if (auto result_tags = tags_map.find(edid); result_tags != tags_map.end())
    {
        auto result_custom_tags = custom_tags_map.find(edid);
        return (result_custom_tags == custom_tags_map.end()) ? &result_tags->second : &result_custom_tags->second;
    }
    else
        return nullptr;
}

bool AnimManager::setTags(std::string_view edid, const StrSet& tags)
{
    if (auto result_tags = tags_map.find(edid); result_tags != tags_map.end())
    {
        custom_tags_map.insert_or_assign(std::string{edid}, tags);
        return true;
    }
    else
        return false;
}

bool AnimManager::loadAnims()
{
    logger::info("Loading animation entries...");

    bool all_ok = true;

    for (auto const& dir_entry : fs::directory_iterator{anim_dir})
        if (dir_entry.is_regular_file())
            if (auto file_path = dir_entry.path(); file_path.extension() == ".json")
            {
                logger::info("Reading {}", file_path.string());

                std::ifstream istream{file_path};
                if (!istream.is_open())
                {
                    logger::warn("Failed to open {}", file_path.filename().string());
                    all_ok = false;
                    continue;
                }

                json j;
                try
                {
                    j = json::parse(istream);
                }
                catch (json::parse_error& e)
                {
                    logParseError(e);
                    all_ok = false;
                    continue;
                }

                StrMap<StrSet> new_tags = {};
                try
                {
                    new_tags = j;
                }
                catch (json::exception& e)
                {
                    logJsonException("StrMap<StrSet>", e);
                    all_ok = false;
                    continue;
                }
                std::erase_if(new_tags, [&](const auto& item) {
                    auto const& [edid, tags] = item;
                    auto form                = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid);
                    if (!form)
                    {
                        logger::warn("Cannot find IdleForm {}!", edid);
                        all_ok = false;
                        return true;
                    }
                    return false;
                });

                auto anim_count = new_tags.size();
                tags_map.merge(new_tags);

                logger::info("Successfully registered {} animations in {}", anim_count, file_path.filename().string());
            }

    logger::info("All animation entries loaded. Total animation count: {}", tags_map.size());
    return all_ok;
}
} // namespace kaputt
