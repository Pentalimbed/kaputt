#include "rules.h"

#include <imgui.h>

namespace kaputt
{
std::unique_ptr<Rule> Rule::getRule(std::string_view rule_name)
{
#define REGRULE(ChildRule)                  \
    if (rule_name == ChildRule().getName()) \
        return std::make_unique<ChildRule>();

    REGRULE(BleedoutRule)
    REGRULE(RagdollRule)
    REGRULE(ProtectedRule)
    REGRULE(EssentialRule)
    REGRULE(AngleRule)
}

// Check
bool AngleRule::operator()(const RE::Actor* attacker, const RE::Actor* victim) const
{
    float rel_angle = const_cast<RE::Actor*>(victim)->GetHeadingAngle(attacker->GetPosition(), false);
    while (rel_angle < params["angle_min"].ref<double>())
        rel_angle += 360;
    while (rel_angle > params["angle_min"].ref<double>())
        rel_angle -= 360;
    return rel_angle >= params["angle_min"].ref<double>();
}

// Draw

void SingleActorRule::drawParams()
{
    bool check_attacker = params["check_attacker"].ref<bool>();
    if (ImGui::Selectable(check_attacker ? "Attacker" : "Victim"))
        params["check_attacker"].ref<bool>() = !check_attacker;
    drawOtherParams();
}

void AngleRule::drawParams()
{
    float angle_min = params["angle_min"].ref<double>();
    float angle_max = params["angle_max"].ref<double>();
    ImGui::SliderAngle("Min Angle", &angle_min);
    ImGui::SliderAngle("Max Angle", &angle_max);
    params["angle_min"].ref<double>() = angle_min;
    params["angle_max"].ref<double>() = angle_max;
}


} // namespace kaputt