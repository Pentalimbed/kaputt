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

std::string joinTags(const StrSet& tags);
StrSet      splitTags(std::string_view str);

inline bool isBetweenAngle(float a, float lb, float ub)
{
    while (a < lb)
        a += 360;
    while (a > ub)
        a -= 360;
    return a >= lb;
}

} // namespace kaputt
