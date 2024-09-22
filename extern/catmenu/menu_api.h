// Copy this file and put alongside imgui headers.

#pragma once

#include <functional>

#include <imgui.h>
#include <ImGuiNotify.hpp>


namespace CatMenu
{

constexpr REL::Version API_VER = {2, 0, 0, 0};

enum class APIResult : uint8_t
{
    OK,
    AlreadyRegistered,
    NotRegistered,
};

class APIBase
{
public:
    virtual REL::Version GetVersion() = 0;

    virtual ImGuiContext* GetContext()                                                      = 0;
    virtual APIResult     RegisterOverlayDrawFunc(const RE::BSString& name, bool (*func)()) = 0;
    virtual APIResult     RegisterMenuDrawFunc(const RE::BSString& name, bool (*func)())    = 0;
    virtual void          InsertNotification(const ImGuiToast& toast)                       = 0;
};

[[nodiscard]] inline std::variant<APIBase*, std::string> RequestCatMenuAPI()
{
    typedef APIBase* (*_RequestCatMenuAPIFunc)();

    auto plugin_handle = GetModuleHandle(L"CatMenu.dll");
    if (!plugin_handle)
        return "Cannot find CatMenu.";

    _RequestCatMenuAPIFunc requestAPIFunc = (_RequestCatMenuAPIFunc)GetProcAddress(plugin_handle, "GetAPI");
    if (requestAPIFunc)
    {
        auto api     = requestAPIFunc();
        auto api_ver = api->GetVersion();
        if (api_ver == API_VER)
            return api;
        else
            return std::format("Version mismatch! Requested {}. Get {}.", API_VER, api_ver);
    }

    return "Failed to get.";
}

} // namespace CatMenu