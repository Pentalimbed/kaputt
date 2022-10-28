#include "kaputt.h"

namespace kaputt
{
bool Kaputt::precondition(const RE::Actor* attacker, const RE::Actor* victim)
{
}
bool Kaputt::submit(
    RE::Actor*                          attacker,
    RE::Actor*                          victim,
    std::set<std::string, std::less<>>* required_tags,
    std::set<std::string, std::less<>>* banned_tags)
{
}
} // namespace kaputt