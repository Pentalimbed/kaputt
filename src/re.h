#pragma once

// Game related utitlies

/* -------------- RELOCATION -------------- */

inline bool playPairedIdle(RE::AIProcess* proc, RE::Actor* attacker, RE::DEFAULT_OBJECT smth, RE::TESIdleForm* idle, bool a5, bool a6, RE::TESObjectREFR* target)
{
    using func_t = decltype(&playPairedIdle);
    REL::Relocation<func_t> func{RELOCATION_ID(38290, 39256)};
    return func(proc, attacker, smth, idle, a5, a6, target);
}

/* ---------------- CUSTOM ---------------- */

RE::Actor* getNearestActor(RE::Actor* origin, float max_range = 256); // Player not included