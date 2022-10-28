#pragma once

#include "kaputtAPI.h"
#include "rule.h"

namespace kaputt
{
class Kaputt : public KaputtAPI
{
private:
    std::atomic_bool ready;

public:
    inline void         set_ready(bool val = true) { ready.store(val); }
    virtual inline bool is_ready() { return ready.load(); }

    virtual bool precondition(const RE::Actor* attacker, const RE::Actor* victim);
    virtual bool submit(
        RE::Actor*                          attacker,
        RE::Actor*                          victim,
        std::set<std::string, std::less<>>* required_tags = nullptr,
        std::set<std::string, std::less<>>* banned_tags   = nullptr);
};
} // namespace kaputt