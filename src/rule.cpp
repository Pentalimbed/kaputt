#include "rule.h"

#include "utils.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace kaputt
{

#define RULEITEM(type) \
    rule_map.insert_or_assign(std::string(type().getName()), std::make_unique<type>())

const StrMap<std::unique_ptr<RuleBase>>& getRule(std::string_view type)
{
    static StrMap<std::unique_ptr<RuleBase>> rule_map = {};
    static std::once_flag                    flag;
    std::call_once(flag, [&]() {
        RULEITEM(UnconditionalRule);
        RULEITEM(BleedoutRule);
        RULEITEM(RagdollRule);
        RULEITEM(ProtectedRule);
        RULEITEM(EssentialRule);
        RULEITEM(AngleRule);
    });
}

void UnconditionalRuleParams::draw() { ImGui::Checkbox(value ? "true" : "false", &value); }

void SingleActorRuleParams::draw()
{
    if (ImGui::Selectable(check_attacker ? "> attacker" : "> victim"))
        check_attacker = !check_attacker;
}

void AngleRuleParams::draw()
{
    ImGui::SliderAngle("min", &angle_min);
    ImGui::SliderAngle("max", &angle_max);
}
bool AngleRule::check(const AngleRuleParams& params, const RE::Actor* attacker, const RE::Actor* victim)
{
    return isBetweenAngle(const_cast<RE::Actor*>(victim)->GetHeadingAngle(attacker->GetPosition(), false), params.angle_min, params.angle_max);
}
} // namespace kaputt