#pragma once

// Game related utitlies

namespace kaputt
{
/* ---------------- HOOKS ---------------- */
struct ProcessHitHook
{
    static void                             thunk(RE::Actor* a_victim, RE::HitData& a_hitData);
    static REL::Relocation<decltype(thunk)> func;

    static constexpr auto id     = REL::RelocationID(37673, 38627);
    static constexpr auto offset = REL::VariantOffset(0x3c, 0x4a, 0x0); // VR Unknown
};

/* ------------- ENGINE FUNC ------------- */

inline bool _playPairedIdle(RE::AIProcess* proc, RE::Actor* attacker, RE::DEFAULT_OBJECT smth, RE::TESIdleForm* idle, bool a5, bool a6, RE::TESObjectREFR* target)
{
    using func_t = decltype(&_playPairedIdle);
    REL::Relocation<func_t> func{RELOCATION_ID(38290, 39256)};
    return func(proc, attacker, smth, idle, a5, a6, target);
}

/* ------------- CUSTOM FUNC ------------- */

RE::Actor* getNearestNPC(RE::Actor* origin, float max_range = 256);

void playPairedIdle(RE::TESIdleForm* idle, RE::Actor* attacker, RE::Actor* victim);
void testPlayPairedIdle(RE::TESIdleForm* idle, float max_range = 25);
} // namespace kaputt
