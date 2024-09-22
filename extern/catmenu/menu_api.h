/*
    MIT License

    Copyright (c) 2024 FiveLimbedCat/ProfJack

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

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