/**
 * @file rules.h
 * @author Cat
 * @brief Rules are basically function wrappers with serializable and ImGui-drawable parameters.
 * @version 0.1
 * @date 2022-10-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "utils.h"

namespace kaputt
{
struct Rule
{
    toml::table         params;
    virtual toml::table getDefaultParams() = 0;

    inline void init() { params = getDefaultParams(); }
    inline void init(const toml::table& p) { params = p; }
    inline bool checkParamsValidity(const toml::table& p) { return isSameStructure(p, getDefaultParams()); }

    virtual void                       drawParams()    = 0;
    virtual constexpr std::string_view getName() const = 0;
    virtual constexpr std::string_view getHint() const = 0;

    virtual bool operator()(const RE::Actor* attacker, const RE::Actor* victim) const = 0; // actual rule checking part

    static std::unique_ptr<Rule> getRule(std::string_view rule_name);
};

struct SingleActorRule : public Rule
{
    virtual inline toml::table getDefaultParams()
    {
        auto def_param = getOtherDefaultParams();
        def_param.emplace<bool>("check_attacker", false);
        return def_param;
    }
    virtual toml::table getOtherDefaultParams() = 0;

    virtual void drawParams();
    virtual void drawOtherParams() = 0;

    virtual bool        operator()(const RE::Actor* actor) const = 0;
    virtual inline bool operator()(const RE::Actor* attacker, const RE::Actor* victim) const { params["check_attacker"].ref<bool>() ? (*this)(attacker) : (*this)(victim); }
};

struct UnconditionalRule : public Rule
{
    virtual inline toml::table getDefaultParams() { return {}; }
    virtual void               drawParams();

    virtual constexpr std::string_view getName() const { return "Always"; }
    virtual constexpr std::string_view getHint() const { return "Always True."; }
    virtual inline bool                operator()(const RE::Actor* attacker, const RE::Actor* victim) const { return true; };
};

struct BleedoutRule : public SingleActorRule
{
    virtual inline toml::table getOtherDefaultParams() { return {}; };
    virtual inline void        drawOtherParams(){};

    virtual constexpr std::string_view getName() const { return "Bleedout"; }
    virtual constexpr std::string_view getHint() const { return "True if actor is bleeding out."; }
    virtual inline bool                operator()(const RE::Actor* actor) const { return actor->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kInBleedoutAnimation); }
};

struct RagdollRule : public SingleActorRule
{
    virtual inline toml::table getOtherDefaultParams() { return {}; };
    virtual inline void        drawOtherParams(){};

    virtual constexpr std::string_view getName() const { return "Ragdoll"; }
    virtual constexpr std::string_view getHint() const { return "True if actor is ragdolling."; }
    virtual inline bool                operator()(const RE::Actor* actor) const
    {
        return actor->IsInRagdollState(); // Alternate method: check knockState
    }
};

struct ProtectedRule : public SingleActorRule
{
    virtual inline toml::table getOtherDefaultParams() { return {}; };
    virtual inline void        drawOtherParams(){};

    virtual constexpr std::string_view getName() const { return "Protected"; }
    virtual constexpr std::string_view getHint() const { return "True if actor is protected."; }
    virtual inline bool                operator()(const RE::Actor* actor) const { return actor->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kProtected); }
};

struct EssentialRule : public SingleActorRule
{
    virtual inline toml::table getOtherDefaultParams() { return {}; };
    virtual inline void        drawOtherParams(){};

    virtual constexpr std::string_view getName() const { return "Essential"; }
    virtual constexpr std::string_view getHint() const { return "True if actor is essential."; }
    virtual inline bool                operator()(const RE::Actor* actor) const { return actor->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kEssential); }
};

struct AngleRule : public Rule
{
    virtual inline toml::table getDefaultParams() { return toml::table{{"angle_min", -45.f}, {"angle_max", 45.f}}; }
    virtual void               drawParams();

    virtual constexpr std::string_view getName() const { return "Attacker Angle"; }
    virtual constexpr std::string_view getHint() const { return "True if the attacker is between 2 angles.\n Ranges from -360 to 360 deg clockwise, 0 being straight ahead."; }
    virtual bool                       operator()(const RE::Actor* attacker, const RE::Actor* victim) const;
};

//////// TAG EQUIVALANCE TODO

} // namespace kaputt