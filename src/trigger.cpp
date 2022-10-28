#include "trigger.h"

#include "re.h"
#include <effolkronium/random.hpp>

namespace kaputt
{
bool PostHitTrigger::process(RE::Actor* victim, RE::HitData& hit_data)
{
    if (!enabled)
        return true;

    auto kap = Kaputt::getSingleton();
    if (!kap->isReady())
        return true;

    auto attacker = hit_data.aggressor.get().get();
    if (!attacker || !victim)
        return true;

    logger::debug("{} hitting {}", attacker->GetName(), victim->GetName());
    if (!kap->precondition(attacker, victim))
        return true;
    logger::debug("precond pass!");

    // execution check
    uint8_t do_trigger = enable_bleedout_execution && isBleedout(victim); // 0-no 1-exec 2-killmove
    // damage check
    if (!do_trigger)
    {
        float dmg_mult = getDamageMult(victim->IsPlayerRef());
        if (victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) <= hit_data.totalDamage * dmg_mult)
            do_trigger = 2;
    }

    if (!do_trigger)
        return true;

    logger::debug("triggering");
    if (!lottery(attacker, victim, do_trigger == 1))
        return true;

    return !kap->submit(attacker, victim); // TODO if animation not played, cache and register hit after some milliseconds
}

bool PostHitTrigger::lottery(RE::Actor* attacker, RE::Actor* victim, bool is_exec)
{
    size_t idx  = -1 + 2 * !attacker->IsPlayerRef() + !victim->IsPlayerRef();
    float  prob = (is_exec ? prob_exec : prob_km)[idx];
    return effolkronium::random_static::get(0.f, 1.f) < prob;
}
} // namespace kaputt