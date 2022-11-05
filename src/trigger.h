#pragma once

#include "kaputt.h"

namespace kaputt
{

class VanillaTrigger
{
public:
    // PARAMS
    bool enabled = true;

    bool                 enable_bleedout_execution = false;
    bool                 enable_getup_execution    = false;
    std::array<float, 3> prob_km                   = {50.f, 50.f, 50.f}; // p2n, n2p, n2n
    std::array<float, 3> prob_exec                 = {50.f, 50.f, 50.f};

    // FUNC
    static VanillaTrigger* getSingleton()
    {
        static VanillaTrigger trigger;
        return std::addressof(trigger);
    }

    bool process(RE::TESActionData* action_data); // for npc
    void process();                               // for player
private:
    bool process(RE::Actor* attacker, RE::Actor* victim);
    bool lottery(RE::Actor* attacker, RE::Actor* victim, bool is_exec);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(VanillaTrigger, enabled, enable_bleedout_execution, enable_getup_execution, prob_km, prob_exec)

class PostHitTrigger
{
public:
    // PARAMS
    bool enabled = false;

    bool                 enable_bleedout_execution = false;
    bool                 enable_getup_execution    = false;
    bool                 instakill                 = false;
    std::array<float, 3> prob_km                   = {100.f, 100.f, 100.f}; // p2n, n2p, n2n
    std::array<float, 3> prob_exec                 = {100.f, 100.f, 100.f};

    // FUNC
    static PostHitTrigger* getSingleton()
    {
        static PostHitTrigger trigger;
        return std::addressof(trigger);
    }

    bool process(RE::Actor* victim, RE::HitData& hit_data);

private:
    bool lottery(RE::Actor* attacker, RE::Actor* victim, bool is_exec);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PostHitTrigger, enabled, enable_bleedout_execution, enable_getup_execution, instakill, prob_km, prob_exec)

class SneakTrigger
{
public:
    // PARAMS
    bool enabled = false;

    bool     need_crouch  = true;
    uint32_t key_scancode = 256; // LMOUSE

    // FUNC
    static SneakTrigger* getSingleton()
    {
        static SneakTrigger trigger;
        return std::addressof(trigger);
    }

    void process(uint32_t scancode);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SneakTrigger, enabled, need_crouch, key_scancode)
} // namespace kaputt