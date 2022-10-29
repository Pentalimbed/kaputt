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
inline RE::TESObjectREFR* _getEquippedShield(RE::Actor* a_actor)
{
    using func_t = decltype(&_getEquippedShield);
    REL::Relocation<func_t> func{RELOCATION_ID(37624, 38577)};
    return func(a_actor);
}

/* ------------- TESCOND WRAPPER ------------- */

bool isInPairedAnimation(const RE::Actor* actor);
bool getDetected(const RE::Actor* attacker, const RE::Actor* victim);

/* ------------- CUSTOM FUNC ------------- */

RE::Actor* getNearestNPC(RE::Actor* origin, float max_range = 256);

void playPairedIdle(RE::TESIdleForm* idle, RE::Actor* attacker, RE::Actor* victim);
void testPlayPairedIdle(RE::TESIdleForm* idle, float max_range = 256);

inline float getDamageMult(bool is_victim_player)
{
    auto              difficulty   = RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
    const std::vector diff_str     = {"VE", "E", "N", "H", "VH", "L"};
    auto              setting_name = fmt::format("fDiffMultHP{}PC{}", is_victim_player ? "To" : "By", diff_str[difficulty]);
    auto              setting      = RE::GameSettingCollection::GetSingleton()->GetSetting(setting_name.c_str());
    return setting->data.f;
}

std::string getSkeletonRace(const RE::Actor* actor);
std::string getEquippedTag(const RE::Actor* actor, bool is_left);

bool canDecap(const RE::Actor* actor);
} // namespace kaputt
