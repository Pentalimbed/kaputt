#pragma once

// Engine address related bs

typedef void(_fastcall* _playPairedIdle)(RE::AIProcess* proc, RE::Actor* attacker, RE::DEFAULT_OBJECT smth, RE::TESIdleForm* idle, bool a5, bool a6, RE::TESObjectREFR* target);
inline REL::Relocation<_playPairedIdle> playPairedIdle{REL::RelocationID(38290, 39256)};