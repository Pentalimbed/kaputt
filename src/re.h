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

struct AttackActionHook
{
    static bool                                    thunk(RE::TESActionData* a_actionData);
    static inline REL::Relocation<decltype(thunk)> func;

    static constexpr auto id     = RELOCATION_ID(48139, 49170);
    static constexpr auto offset = REL::VariantOffset(0x4d7, 0x435, 0x0); // VR Unknown
};

/* ---------------- EVENT ---------------- */

class InputEventSink : public RE::BSTEventSink<RE::InputEvent*>
{
public:
    virtual EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource);
    static void         RegisterSink()
    {
        static InputEventSink _sink;
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(&_sink);
    }
};

class PlayerAnimGraphEventSink : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    virtual EventResult ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);
    static void         RegisterSink()
    {
        static PlayerAnimGraphEventSink _sink;
        RE::PlayerCharacter::GetSingleton()->AddAnimationGraphEventSink(&_sink);
    }
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
bool isFurnitureAnimType(const RE::Actor* actor, RE::BSFurnitureMarker::AnimationType type);
bool shouldAttackKill(const RE::Actor* attacker, const RE::Actor* victim);

/* ------------- CUSTOM FUNC ------------- */

inline bool isGamePaused()
{
    auto UI = RE::UI::GetSingleton();
    return !UI || UI->GameIsPaused();
}

inline bool animPlayable(const RE::Actor* actor)
{
    return actor->Is3DLoaded() && !actor->IsDisabled() && !actor->IsDead() && !isInPairedAnimation(actor) && !actor->IsOnMount() && !actor->IsInRagdollState();
}

RE::Actor* getNearestNPC(RE::Actor* origin, float max_range = 256);
bool       isLastHostileInRange(const RE::Actor* attacker, const RE::Actor* victim, float range);

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

} // namespace kaputt
