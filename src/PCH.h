#pragma once

#define UNICODE
#define _UNICODE
#define WIN32_MEAN_AND_LEAN

#define TOML_EXCEPTIONS 0

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace logger = SKSE::log;
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

namespace kaputt
{
constexpr auto plugin_dir      = "Data\\SKSE\\Plugins";
constexpr auto config_dir      = "Kaputt";
constexpr auto anim_dir        = "animations";
constexpr auto anim_custom_dir = "custom";
constexpr auto anim_custom_def = "default.toml";
constexpr auto filter_dir      = "filters";
constexpr auto filter_def      = "default.toml";
} // namespace kaputt

using EventResult = RE::BSEventNotifyControl;