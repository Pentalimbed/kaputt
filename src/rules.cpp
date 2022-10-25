#include "rules.h"

#include <imgui.h>

namespace kaputt
{

// Check
bool AngleRule::operator()(RE::Actor* attacker, RE::Actor* victim) const
{
    float rel_angle = victim->GetHeadingAngle(attacker->GetPosition(), false);
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