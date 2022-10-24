#include "animation.h"

#include <sstream>

StrSet tomlArray2Tags(const toml::array* arr)
{
    StrSet tags;
    for (const auto& tag : *arr)
        if (tag.is_string())
            tags.insert(tag.ref<std::string>());
    return tags;
}

/*UNUSED*/
StrSet splitTags(const std::string& str)
{
    std::istringstream iss(str);
    std::string        temp_tag;
    StrSet             tags;
    while (std::getline(iss, temp_tag, ' '))
        tags.insert(temp_tag);
    return tags;
}

std::string joinTags(const StrSet& tags)
{
    std::string result = "";
    uint16_t    count  = 0;
    for (auto it = tags.begin(); it != tags.end(); ++it, ++count)
    {
        result += *it;
        if (count != tags.size() - 1)
            result += ' ';
    }
    return result;
}

namespace kaputt
{
void AnimEntry::parse_toml_array(const toml::array* arr, bool is_custom)
{
    if (is_custom)
        custom_tags.emplace();
    for (const auto& tag : *arr)
        if (tag.is_string())
            (is_custom ? custom_tags.value() : tags).insert(tag.ref<std::string>());
}

void AnimEntryManager::loadSingleEntryFile(fs::path dir)
{
    auto result = toml::parse_file(dir.c_str());
    if (!result)
    {
        auto err = result.error();
        logger::warn("Failed to parse file {}. Error: {} (Line {}, Col {})",
                     dir.string(), err.description(), err.source().begin.line, err.source().begin.column);
        return;
    }
    logger::info("Parsing file {}", dir.string());

    auto     tbl        = result.table();
    uint16_t anim_count = 0;
    for (auto [edid, v] : tbl)
        if (v.is_array())
        {
            auto form = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(edid);
            if (!form)
            {
                logger::warn("Failed to parse entry {}. Error: Cannot find an IdleForm with this EditorId!", edid);
                continue;
            }
            if (anim_dict.contains(edid.str()))
            {
                logger::warn("Failed to parse entry {}. Error: An entry with the same EditorId has been registered!", edid);
                continue;
            }

            try
            {
                AnimEntry entry{
                    .editor_id = std::string(edid),
                    .idle_form = form,
                };
                entry.parse_toml_array(v.as_array());
                anim_dict[std::string(edid)] = std::move(entry);
                logger::info("Added entry {}.", edid);
                ++anim_count;
            }
            catch (std::runtime_error e)
            {
                logger::warn("Failed to parse entry {}. Error: {}", edid, e.what());
            }
        }
    logger::info("Parsed file {}. {} animations were registered.", dir.string(), anim_count);
}

void AnimEntryManager::loadAllEntryFiles()
{
    logger::info("Reading animations...");

    auto full_anim_dir = fs::path(plugin_dir) / fs::path(config_dir) / fs::path(anim_dir);
    for (auto const& dir_entry : fs::directory_iterator{full_anim_dir})
        if (dir_entry.is_regular_file())
            if (auto file_path = dir_entry.path(); file_path.extension() == ".toml")
                loadSingleEntryFile(file_path);
}

void AnimEntryManager::loadCustomFile(fs::path dir)
{
    auto result = toml::parse_file(dir.c_str());
    if (!result)
    {
        auto err = result.error();
        logger::warn("Failed to parse file {}. Error: {} (Line {}, Col {}).",
                     dir.string(), err.description(), err.source().begin.line, err.source().begin.column);
        return;
    }
    logger::info("Parsing customization file {}", dir.string());

    auto tbl = result.table();
    for (auto [edid, v] : tbl)
        if (v.is_array())
        {
            if (!anim_dict.contains(edid.str()))
            {
                logger::info("Customization entry {} is not registered. This is ok.", edid);
                continue;
            }

            try
            {
                anim_dict[std::string(edid)].parse_toml_array(v.as_array(), true);
                logger::info("Added customization entry {}", edid);
            }
            catch (std::runtime_error e)
            {
                logger::warn("Failed to parse customization entry {}. Error: {}", edid, e.what());
            }
        }
    logger::info("Parsed customization file {}.", dir.string());
}

void AnimEntryManager::saveCustomFile(fs::path dir)
{
    logger::info("Saving customization file {}.", dir.string());

    std::ofstream f(dir);
    if (!f.is_open())
    {
        logger::error("Failed to write at {}!", dir.string());
        return;
    }

    toml::table tbl = {};
    for (const auto& [edid, anim] : anim_dict)
        if (anim.custom_tags.has_value())
        {
            auto arr = tbl.emplace<toml::array>(edid).first->second.as_array();
            for (const auto& tag : anim.custom_tags.value())
                arr->push_back(tag);
        }
    f << tbl;
    logger::info("Saved customization file {}.", dir.string());
}

void AnimEntryManager::clearCustomTags()
{
    for (auto& [edid, anim] : anim_dict)
        anim.custom_tags = std::nullopt;
    logger::info("All custom tags cleared.");
}
} // namespace kaputt