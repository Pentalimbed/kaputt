#pragma once

// Game related utitlies

namespace kaputt
{
/* ---------------- HOOKS ---------------- */
struct ProcessHitHook
{
    static void                                    thunk(RE::Actor* a_victim, RE::HitData& a_hitData);
    static inline REL::Relocation<decltype(thunk)> func;

    static constexpr auto id     = RELOCATION_ID(37673, 38627);
    static constexpr auto offset = REL::VariantOffset(0x3c0, 0x4a8, 0x0); // VR Unknown
};

/* ------------- ENGINE FUNC ------------- */

inline bool _playPairedIdle(RE::AIProcess* proc, RE::Actor* attacker, RE::DEFAULT_OBJECT smth, RE::TESIdleForm* idle, bool a5, bool a6, RE::TESObjectREFR* target)
{
    using func_t = decltype(&_playPairedIdle);
    REL::Relocation<func_t> func{RELOCATION_ID(38290, 39256)};
    return func(proc, attacker, smth, idle, a5, a6, target);
}

/* ------------- TESCOND WRAPPER ------------- */

bool isInPairedAnimation(const RE::Actor* actor);

/* ------------- CUSTOM FUNC ------------- */

inline bool isBleedout(const RE::Actor* actor) { return actor->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kInBleedoutAnimation); }

RE::Actor* getNearestNPC(RE::Actor* origin, float max_range = 256);

void playPairedIdle(RE::TESIdleForm* idle, RE::Actor* attacker, RE::Actor* victim);
void testPlayPairedIdle(RE::TESIdleForm* idle, float max_range = 256);

inline float getDamageMult(bool is_player)
{
    auto              difficulty   = RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
    const std::vector diff_str     = {"VE", "E", "N", "H", "VH", "L"};
    auto              setting_name = fmt::format("fDiffMultHP{}PC{}", is_player ? "To" : "By", diff_str[difficulty]);
    auto              setting      = RE::GameSettingCollection::GetSingleton()->GetSetting(setting_name.c_str());
    return setting->data.f;
}
} // namespace kaputt
