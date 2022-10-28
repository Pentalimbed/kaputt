#pragma once

#include "filter.h"
#include "animation.h"

namespace kaputt
{
class Kaputt
{
private:
    std::atomic_bool ready;

    std::vector<RuleInfo> preconds = {};

public:
    AnimManager    anim_manager;
    FilterPipeline filter;

    static Kaputt* getSingleton()
    {
        static Kaputt kaputt;
        return std::addressof(kaputt);
    }
    bool                init();
    virtual inline bool isReady() { return ready.load(); }

    virtual inline bool precondition(const RE::Actor* attacker, const RE::Actor* victim)
    {
        return std::ranges::all_of(preconds, [=](RuleInfo& rule) { return rule.enabled == rule.check(attacker, victim); });
    }
    virtual bool submit(
        RE::Actor*          attacker,
        RE::Actor*          victim,
        const TaggerOutput& extra_tags = {});

    bool loadConfig(std::string_view dir);
    bool saveConfig(std::string_view dir);
};
} // namespace kaputt