#pragma once

#include "kaputt.h"

namespace kaputt
{

class PostHitTrigger
{
public:
    // PARAMS
    bool enabled = true;

    bool                 enable_bleedout_execution = true;
    std::array<float, 3> prob_km                   = {1.0f, 0.0f, 1.0f}; // p2n, n2p, n2n
    std::array<float, 3> prob_exec                 = {1.0f, 0.0f, 1.0f};

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
} // namespace kaputt