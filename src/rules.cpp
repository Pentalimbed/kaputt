#include "rules.h"

#include <imgui.h>

namespace kaputt
{

std::vector<std::string_view> Rule::listRules()
{
    return {UnconditionalRule().getName(),
            BleedoutRule().getName(),
            RagdollRule().getName(),
            ProtectedRule().getName(),
            EssentialRule().getName(),
            AngleRule().getName()};
}

std::unique_ptr<Rule> Rule::getRule(std::string_view rule_name)
{
#define REGRULE(ChildRule)                           \
    if (rule_name == ChildRule().getName())          \
    {                                                \
        auto result = std::make_unique<ChildRule>(); \
        result->init();                              \
        return std::move(result);                    \
    }

    REGRULE(UnconditionalRule)
    REGRULE(BleedoutRule)
    REGRULE(RagdollRule)
    REGRULE(ProtectedRule)
    REGRULE(EssentialRule)
    REGRULE(AngleRule)

    logger::error("The plugin is trying to request a nonexistant rule. Please report to the author.");
    return nullptr;
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
    if (ImGui::Selectable(check_attacker ? "On Attacker" : "On Victim"))
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