#include "utils.h"

std::string joinTags(const StrSet& tags, bool sorted)
{
    std::string result = "";
    uint16_t    count  = 0;
    if (sorted)
    {
        std::vector<std::string_view> taglist(tags.begin(), tags.end());
        std::ranges::sort(taglist);
        for (auto it = taglist.begin(); it != taglist.end(); ++it, ++count)
        {
            result += *it;
            if (count != taglist.size() - 1)
                result += ' ';
        }
    }
    else
    {
        for (auto it = tags.begin(); it != tags.end(); ++it, ++count)
        {
            result += *it;
            if (count != tags.size() - 1)
                result += ' ';
        }
    }

    return result;
}

StrSet splitTags(const std::string& str)
{
    std::istringstream iss(str);
    std::string        temp_tag;
    StrSet             tags;
    while (std::getline(iss, temp_tag, ' '))
        tags.insert(temp_tag);
    return tags;
}