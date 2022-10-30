#include "rule.h"

#include "utils.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <effolkronium/random.hpp>

namespace kaputt
{

#define RULEITEM(type) \
    rule_map.insert_or_assign(std::string(type().getName()), std::make_unique<type>())

const StrMap<std::shared_ptr<RuleBase>>& getRule()
{
    static StrMap<std::shared_ptr<RuleBase>> rule_map = {};
    static std::once_flag                    flag;
    std::call_once(flag, [&]() {
        RULEITEM(UnconditionalRule);
        RULEITEM(PlayableRule);
        RULEITEM(BleedoutRule);
        RULEITEM(RagdollRule);
        RULEITEM(ProtectedRule);
        RULEITEM(EssentialRule);
        RULEITEM(AngleRule);
        RULEITEM(LastHostileInRangeRule);
        RULEITEM(SkeletonRule);
        RULEITEM(PlayerRule);
        RULEITEM(RaceRule);
        RULEITEM(PerkRule);
        RULEITEM(DecapPerkRule);
        RULEITEM(SneakRule);
        RULEITEM(DetectedRule);
        RULEITEM(FactionRule);
    });
    return rule_map;
}

void UnconditionalRuleParams::draw() { ImGui::Checkbox(value ? "true" : "false", &value); }

void SingleActorRuleParams::draw()
{
    if (ImGui::Selectable(check_attacker ? "> attacker" : "> victim"))
        check_attacker = !check_attacker;
}

void AngleRuleParams::draw()
{
    ImGui::SliderFloat("min", &angle_min, -360, 360, "%.0f deg");
    ImGui::SliderFloat("max", &angle_max, -360, 360, "%.0f deg");
}
bool AngleRule::check(const AngleRuleParams& params, const RE::Actor* attacker, const RE::Actor* victim)
{
    return isBetweenAngle(const_cast<RE::Actor*>(victim)->GetHeadingAngle(attacker->GetPosition(), false), params.angle_min, params.angle_max);
}

void LastHostileInRangeRuleParams::draw()
{
    ImGui::SliderFloat("range", &range, 0.f, 4096.f);
}
bool LastHostileInRangeRule::check(const LastHostileInRangeRuleParams& params, const RE::Actor* attacker, const RE::Actor* victim)
{
    auto process_lists = RE::ProcessLists::GetSingleton();
    if (!process_lists)
    {
        logger::error("Failed to get ProcessLists!");
        return false;
    }
    auto n_load_actors = process_lists->numberHighActors;
    if (n_load_actors == 0)
        return true;

    for (auto actor_handle : process_lists->highActorHandles)
    {
        if (!actor_handle || !actor_handle.get())
            continue;

        auto actor = actor_handle.get().get();

        if ((actor == attacker) || (actor == victim) || (actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) <= 0))
            continue;

        float dist = actor->GetPosition().GetDistance(attacker->GetPosition());
        if ((dist < params.range) && actor->IsHostileToActor(const_cast<RE::Actor*>(attacker)))
            return false;
    }
    // EXTRA: CHECK PLAYER

    if (!attacker->IsPlayerRef() && !victim->IsPlayerRef())
        if (RE::Actor* player = RE::PlayerCharacter::GetSingleton(); player)
        {
            float dist = player->GetPosition().GetDistance(attacker->GetPosition());
            if ((dist < params.range) && const_cast<RE::Actor*>(attacker)->IsHostileToActor(player))
                return false;
        }

    return true;
}

void SkeletonRuleParams::draw()
{
    SingleActorRuleParams::draw();
    ImGui::InputText("skeleton", &skeleton);
}
bool SkeletonRule::check(const SkeletonRuleParams& params, const RE::Actor* attacker, const RE::Actor* victim)
{
    auto actor = (params.check_attacker ? attacker : victim);
    return strcmpi(actor->GetRace()->skeletonModels[actor->GetActorBase()->IsFemale()].model.c_str(),
                   params.skeleton.c_str()) == 0;
}

void RaceRuleParams::draw()
{
    SingleActorRuleParams::draw();
    ImGui::InputText("race", &race);
}

void PerkRuleParams::draw()
{
    SingleActorRuleParams::draw();
    ImGui::InputText("perk", &perk);
}

void DecapPerkRuleParams::draw()
{
    SingleActorRuleParams::draw();
    ImGui::SliderFloat("chance", &decap_chance, 0.f, 100.f, "%.0f %%");
}
bool DecapPerkRule::check(const DecapPerkRuleParams& params, const RE::Actor* attacker, const RE::Actor* victim)
{
    bool can_decap = canDecap(params.check_attacker ? attacker : victim);
    return can_decap ? (effolkronium::random_static::get(0.f, 100.f) < params.decap_chance) : false;
}

void FactionRuleParams::draw()
{
    SingleActorRuleParams::draw();
    ImGui::InputText("faction", &faction);
}
} // namespace kaputt