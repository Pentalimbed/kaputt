#include "re.h"

RE::Actor* getNearestActor(RE::Actor* origin, float max_range)
{
    logger::debug("getNearestActor");
    auto process_lists = RE::ProcessLists::GetSingleton();
    if (!process_lists)
    {
        logger::error("Failed to get ProcessLists!");
        return nullptr;
    }
    auto n_load_actors = process_lists->numberHighActors;
    logger::debug("Number of high actors: {}.", n_load_actors);
    if (n_load_actors == 0)
        return nullptr;

    float      min_dist  = FLT_MAX;
    RE::Actor* min_actor = nullptr;
    for (auto actor_handle : process_lists->highActorHandles)
    {
        auto actor = actor_handle.get().get();
        logger::debug("Checking actor: {}", actor->GetName());
        if (actor == origin)
            continue;

        float dist = actor->GetPosition().GetDistance(origin->GetPosition());
        if (dist < min_dist)
        {
            min_dist  = dist;
            min_actor = actor;
        }
    }

    if (!min_actor)
        logger::debug("No actor in range.");
    return min_actor;
}