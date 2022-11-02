#include "utils.h"

#include <dinput.h>

#include <imgui.h>
#include "imgui_stdlib.h"

namespace kaputt
{
bool drawTagsInputText(std::string_view label, StrSet& tags)
{
    auto tagstr = joinTags(tags);
    auto result = ImGui::InputText(label.data(), &tagstr);
    if (result)
        tags = splitTags(tagstr);
    return result;
}

void mergeSet(StrSet& from, const StrSet& to)
{
    for (auto& item : to)
        from.emplace(item);
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

std::string scanCode2String(uint32_t scancode)
{
    if (scancode >= kGamepadOffset)
    {
        // not implemented
        return "";
    }
    else if (scancode >= kMouseOffset)
    {
        auto key = scancode - kMouseOffset;
        switch (key)
        {
            case 0:
                return "Left Mouse Button";
            case 1:
                return "Right Mouse Button";
            case 2:
                return "Middle Mouse Button";
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                return fmt::format("Mouse Button {}", key);
            case 8:
                return "Wheel Up";
            case 9:
                return "Wheel Down";
            default:
                return "";
        }
    }
    else
    {
        TCHAR lpszName[256];
        if (GetKeyNameText(scancode << 16, lpszName, sizeof(lpszName)))
        {
            int         size_needed = WideCharToMultiByte(CP_UTF8, 0, lpszName, wcslen(lpszName), NULL, 0, NULL, NULL);
            std::string key_str(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, lpszName, wcslen(lpszName), &key_str[0], size_needed, NULL, NULL);
            return key_str;
        }
        else
        {
            return "";
        }
    }
}
} // namespace kaputt