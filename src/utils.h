#pragma once

#include <nlohmann/json.hpp>

namespace kaputt
{
using json = nlohmann::json;

inline void logParseError(const json::parse_error& e)
{
    logger::warn("Parse error at input byte {}\n"
                 "\t{}",
                 e.byte, e.what());
}

inline void logJsonException(std::string_view context, const json::exception& e)
{
    logger::warn("Error while deserializing {}\n"
                 "\t{}",
                 context, e.what());
}

void mergeSet(StrSet& from, const StrSet& to);

bool        drawTagsInputText(std::string_view label, StrSet& tags);
std::string joinTags(const StrSet& tags);
StrSet      splitTags(std::string_view str);

inline bool isBetweenAngle(float a, float lb, float ub)
{
    while (a >= lb)
        a -= 360;
    while (a <= lb)
        a += 360;
    return a <= ub;
}

enum : uint32_t
{
    kInvalid        = static_cast<uint32_t>(-1),
    kKeyboardOffset = 0,
    kMouseOffset    = 256,
    kGamepadOffset  = 266
};
std::string scanCode2String(uint32_t scancode);

} // namespace kaputt
