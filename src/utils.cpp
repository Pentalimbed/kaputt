#include "utils.h"

namespace kaputt
{
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
StrSet splitTags(std::string_view str)
{
    std::string        temp_str{str};
    std::istringstream iss(temp_str);
    std::string        temp_tag;
    StrSet             tags;
    while (std::getline(iss, temp_tag, ' '))
        tags.insert(temp_tag);
    return tags;
}
} // namespace kaputt