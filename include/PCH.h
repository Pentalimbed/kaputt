#pragma once

#define UNICODE
#define _UNICODE
#define WIN32_MEAN_AND_LEAN

#define TOML_EXCEPTIONS 0

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include "Plugin.h"

namespace logger = SKSE::log;
namespace WinAPI = SKSE::WinAPI;

using namespace std::literals;

namespace stl
{
template <class T>
void write_thunk_call()
{
    auto&                           trampoline = SKSE::GetTrampoline();
    REL::Relocation<std::uintptr_t> hook{T::id, T::offset};
    T::func = trampoline.write_call<5>(hook.address(), T::thunk);
}
} // namespace stl

using EventResult = RE::BSEventNotifyControl;

template <typename T>
using StrMap = std::map<std::string, T, std::less<>>;
using StrSet = std::set<std::string, std::less<>>;

namespace kaputt
{
constexpr auto def_config_path = R"(Data\SKSE\Plugins\kaputt.json)";
constexpr auto config_dir      = R"(Data\SKSE\Plugins\kaputt\configs)";
constexpr auto anim_dir        = R"(Data\SKSE\Plugins\kaputt\anims)";
} // namespace kaputt