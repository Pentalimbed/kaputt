#pragma once

namespace kaputt
{

class AnimManager
{
    friend bool saveConfig(std::string_view path);
    friend bool loadConfig(std::string_view path);

private:
    StrMap<StrSet> tags_map        = {};
    StrMap<StrSet> custom_tags_map = {};

public:
    static AnimManager* getSingleton()
    {
        static AnimManager manager;
        return std::addressof(manager);
    }

    bool loadAnims();

    std::vector<std::string_view> listAnims(std::string_view filter_str = "", int filter_mode = 0);
    inline bool                   hasCustomTags(std::string_view edid) { return custom_tags_map.contains(edid); };
    const StrSet*                 getTags(std::string_view edid);
    bool                          setTags(std::string_view edid, const StrSet& tags);
    inline void                   clearTags(std::string_view edid) { custom_tags_map.erase(edid); }
    inline void                   clearTags() { custom_tags_map.clear(); }
};

} // namespace kaputt
