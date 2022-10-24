#pragma once

#include <robin_hood.h>

struct StrHash
{
    using is_transparent = void;

    inline std::size_t operator()(const std::string& key) const { return robin_hood::hash_bytes(key.c_str(), key.size()); }
    inline std::size_t operator()(std::string_view key) const { return robin_hood::hash_bytes(key.data(), key.size()); }
    inline std::size_t operator()(const char* key) const { return robin_hood::hash_bytes(key, std::strlen(key)); }
};

struct StrEq
{
    using is_transparent = int;

    inline bool operator()(std::string_view lhs, const std::string& rhs) const
    {
        const std::string_view view = rhs;
        return lhs == view;
    }

    inline bool operator()(const char* lhs, const std::string& rhs) const
    {
        return std::strcmp(lhs, rhs.c_str()) == 0;
    }

    inline bool operator()(const std::string& lhs, const std::string& rhs) const
    {
        return lhs == rhs;
    }
};

template <typename T>
using StrMap = robin_hood::unordered_map<std::string, T, StrHash, StrEq>;
using StrSet = robin_hood::unordered_set<std::string, StrHash, StrEq>;