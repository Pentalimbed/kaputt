#pragma once

#include <variant>
#include <Windows.h>

// credit: Ersh

namespace kaputt
{
constexpr REL::Version API_VER = {1, 0, 5, 0};

struct SubmitInfoStruct
{
    std::set<std::string, std::less<>> required_tags = {};
    std::set<std::string, std::less<>> banned_tags   = {};
};

class KaputtAPI
{
public:
    virtual REL::Version getVersion()                                                     = 0;
    virtual bool         isReady()                                                        = 0; // check if everything's loaded
    virtual bool         precondition(const RE::Actor* attacker, const RE::Actor* victim) = 0; // applying kaputt-defined preconditions before examining yourself
    virtual bool         submit(RE::Actor*              attacker,
                                RE::Actor*              victim,
                                const SubmitInfoStruct& submit_info = {})                 = 0; // request playing one of the registered animations with extra tag requirements.
};

[[nodiscard]] inline std::variant<KaputtAPI*, std::string> RequestKaputtAPI()
{
    typedef KaputtAPI* (*_RequestKaputtAPIFunc)();

    auto pluginHandle = GetModuleHandle(L"Kaputt.dll");
    if (!pluginHandle)
        return "Cannot find Kaputt.";

    _RequestKaputtAPIFunc requestAPIFunc = (_RequestKaputtAPIFunc)GetProcAddress(pluginHandle, "GetKaputtInterface");
    if (requestAPIFunc)
    {
        auto api = requestAPIFunc();
        if (api->getVersion() == API_VER)
            return api;
        else
            return std::format("Version mismatch! Requested {}. Get {}.", API_VER, api->getVersion());
    }

    return "Failed to get function GetKaputtInterface.";
}
} // namespace kaputt