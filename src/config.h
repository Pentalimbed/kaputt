#pragma once

namespace kaputt
{
struct Config
{
    static Config* getSingleton()
    {
        static Config config;
        return std::addressof(config);
    }
};
} // namespace kaputt