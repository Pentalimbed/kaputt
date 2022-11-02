#include "trigger.h"

#include "re.h"
#include <effolkronium/random.hpp>

namespace kaputt
{
bool VanillaTrigger::process(RE::TESActionData* action_data)
{
    if (!enabled)
        return true;

    auto kap = Kaputt::getSingleton();
    if (!kap->isReady())
        return true;

    if (!(action_data && action_data->source))
        return true;
    auto attacker = action_data->source->As<RE::Actor>();
    if (!attacker)
        return true;

    auto victim_ptr = attacker->GetActorRuntimeData().currentCombatTarget.get();
    if (!victim_ptr)
        return true;

    return process(attacker, victim_ptr.get());
}

void VanillaTrigger::process()
{
    if (!enabled)
        return;

    auto kap = Kaputt::getSingleton();
    if (!kap->isReady())
        return;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player)
        return;

    auto target_ref = RE::CrosshairPickData::GetSingleton()->targetActor;
    if (!target_ref || !target_ref.get())
        return;

    auto target = target_ref.get()->As<RE::Actor>();
    if (!target)
        return;

    process(player, target);
}

bool VanillaTrigger::process(RE::Actor* attacker, RE::Actor* victim)
{
    logger::debug("VANILLA PROCESSING {} to {}!", attacker->GetName(), victim->GetName());

    // should kill cond
    if (!shouldAttackKill(attacker, victim))
        return true;

    logger::debug("should kill!");

    if (!Kaputt::getSingleton()->precondition(attacker, victim))
        return true;

    if (!lottery(attacker, victim))
        return true;

    Kaputt::getSingleton()->submit(attacker, victim);
    return true;
}

bool VanillaTrigger::lottery(RE::Actor* attacker, RE::Actor* victim)
{
    size_t idx = -1 + 2 * !attacker->IsPlayerRef() + !victim->IsPlayerRef();
    return effolkronium::random_static::get(0.f, 100.f) < prob[idx];
}

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

    // 0-no 1-exec 2-killmove
    // bleedout check
    uint8_t do_trigger = enable_bleedout_execution && victim->AsActorState()->IsBleedingOut();
    // getup check
    bool getting_up = (victim->AsActorState()->GetKnockState() == RE::KNOCK_STATE_ENUM::kGetUp) ||
        (victim->AsActorState()->GetKnockState() == RE::KNOCK_STATE_ENUM::kQueued);
    if (!do_trigger)
        do_trigger = enable_getup_execution && getting_up;
    // damage check
    if (!do_trigger)
    {
        float dmg_mult = getDamageMult(victim->IsPlayerRef());
        if (victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) <= hit_data.totalDamage * dmg_mult)
            do_trigger = 2;
    }

    if (!do_trigger)
        return true;

    logger::debug("posthit triggering!");

    if (!kap->precondition(attacker, victim))
        return true;

    logger::debug("precond pass!");

    if (!lottery(attacker, victim, do_trigger == 1))
        return true;

    if ((do_trigger == 1) && instakill) // instakill operation
        hit_data.totalDamage = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / getDamageMult(victim->IsPlayerRef()) + 10;

    kap->submit(attacker, victim);
    return true;
}

bool PostHitTrigger::lottery(RE::Actor* attacker, RE::Actor* victim, bool is_exec)
{
    size_t idx  = -1 + 2 * !attacker->IsPlayerRef() + !victim->IsPlayerRef();
    float  prob = (is_exec ? prob_exec : prob_km)[idx];
    return effolkronium::random_static::get(0.f, 100.f) < prob;
}

void SneakTrigger::process(uint32_t scancode)
{
    if (!enabled)
        return;

    auto kap = Kaputt::getSingleton();
    if (!kap->isReady())
        return;

    if (scancode != key_scancode)
        return;

    auto target_ref = RE::CrosshairPickData::GetSingleton()->targetActor;
    if (!target_ref || !target_ref.get())
        return;

    auto target = target_ref.get()->As<RE::Actor>();
    if (!target)
        return;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player)
        return;

    // stealth check
    if (getDetected(player, target))
        return;
    if (need_crouch && !player->IsSneaking())
        return;

    if (!kap->precondition(player, target))
        return;

    kap->submit(player, target, need_crouch ? SubmitInfoStruct{} : SubmitInfoStruct{.required_tags = {"sneak"}});
    return;
}
} // namespace kaputt