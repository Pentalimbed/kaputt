#include "re.h"

#include "utils.h"
#include "menu.h"
#include "trigger.h"
#include "tasks.h"

namespace kaputt
{
union ConditionParam
{
    char         c;
    std::int32_t i;
    float        f;
    RE::TESForm* form;
};

void ProcessHitHook::thunk(RE::Actor* a_victim, RE::HitData& a_hitData)
{
    PostHitTrigger::getSingleton()->process(a_victim, a_hitData);
    func(a_victim, a_hitData);
}

bool AttackActionHook::thunk(RE::TESActionData* a_actionData)
{
    return VanillaTrigger::getSingleton()->process(a_actionData) && func(a_actionData);
}

void UpdateHook::thunk(RE::Main* a_this, float a2)
{
    func(a_this, a2);
    TaskManager::getSingleton()->update();
}

EventResult InputEventSink::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource)
{
    if (isGamePaused())
        return RE::BSEventNotifyControl::kContinue;

    if (!a_event || !a_eventSource)
        return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_event; event; event = event->next)
        if (event->eventType == RE::INPUT_EVENT_TYPE::kButton)
        {
            const auto button = static_cast<RE::ButtonEvent*>(event);
            if (!button || !button->IsDown())
                continue;

            auto key = button->GetIDCode();
            switch (button->device.get())
            {
                case RE::INPUT_DEVICE::kMouse:
                    key += kMouseOffset;
                    break;
                case RE::INPUT_DEVICE::kKeyboard:
                    key += kKeyboardOffset;
                    break;
                case RE::INPUT_DEVICE::kGamepad:
                    key += kGamepadOffset;
                    break;
                default:
                    continue;
            }

            SneakTrigger::getSingleton()->process(key);
        }
    return RE::BSEventNotifyControl::kContinue;
}

EventResult PlayerAnimGraphEventSink::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
    if (!a_event || !a_eventSource)
        return RE::BSEventNotifyControl::kContinue;

    // cheese
    std::string tag(a_event->tag);
    std::transform(tag.begin(), tag.end(), tag.begin(), tolower);
    if (tag.contains("attackstart") || tag.contains("attack_start"))
        VanillaTrigger::getSingleton()->process();

    return RE::BSEventNotifyControl::kContinue;
}

bool isInPairedAnimation(const RE::Actor* actor)
{
    static RE::TESConditionItem cond;
    static std::once_flag       flag;
    std::call_once(flag, [&]() {
        cond.data.functionData.function = RE::FUNCTION_DATA::FunctionID::kGetPairedAnimation;
        cond.data.comparisonValue.f     = 0.0f;
        cond.data.flags.opCode          = RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo;
        cond.data.object                = RE::CONDITIONITEMOBJECT::kSelf;
    });

    RE::ConditionCheckParams params(const_cast<RE::TESObjectREFR*>(actor->As<RE::TESObjectREFR>()), nullptr);
    return cond(params);
}

bool getDetected(const RE::Actor* attacker, const RE::Actor* victim)
{
    static RE::TESConditionItem cond;
    static std::once_flag       flag;
    std::call_once(flag, [&]() {
        cond.data.functionData.function = RE::FUNCTION_DATA::FunctionID::kGetDetected;
        cond.data.comparisonValue.f     = 0.0f;
        cond.data.flags.opCode          = RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo;
        cond.data.object                = RE::CONDITIONITEMOBJECT::kTarget;
    });

    RE::ConditionCheckParams params(const_cast<RE::TESObjectREFR*>(attacker->As<RE::TESObjectREFR>()),
                                    const_cast<RE::TESObjectREFR*>(victim->As<RE::TESObjectREFR>()));
    return cond(params);
}

bool isFurnitureAnimType(const RE::Actor* actor, RE::BSFurnitureMarker::AnimationType type)
{
    static RE::TESConditionItem cond;
    static std::once_flag       flag;
    std::call_once(flag, [&]() {
        cond.data.functionData.function = RE::FUNCTION_DATA::FunctionID::kIsFurnitureAnimType;
        cond.data.flags.opCode          = RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
        cond.data.object                = RE::CONDITIONITEMOBJECT::kSelf;
        cond.data.comparisonValue.f     = 1.0f;
    });

    ConditionParam cond_param;
    cond_param.i                     = static_cast<int32_t>(type);
    cond.data.functionData.params[0] = std::bit_cast<void*>(cond_param);

    RE::ConditionCheckParams params(const_cast<RE::TESObjectREFR*>(actor->As<RE::TESObjectREFR>()), nullptr);
    return cond(params);
}

bool shouldAttackKill(const RE::Actor* attacker, const RE::Actor* victim)
{
    static RE::TESConditionItem cond;
    static std::once_flag       flag;
    std::call_once(flag, [&]() {
        cond.data.functionData.function = RE::FUNCTION_DATA::FunctionID::kShouldAttackKill;
        cond.data.flags.opCode          = RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
        cond.data.comparisonValue.f     = 1.0f;
    });

    ConditionParam cond_param;
    cond_param.form                  = const_cast<RE::TESObjectREFR*>(victim->As<RE::TESObjectREFR>());
    cond.data.functionData.params[0] = std::bit_cast<void*>(cond_param);

    RE::ConditionCheckParams params(const_cast<RE::TESObjectREFR*>(attacker->As<RE::TESObjectREFR>()),
                                    const_cast<RE::TESObjectREFR*>(victim->As<RE::TESObjectREFR>()));
    return cond(params);
}


bool isLastHostileInRange(const RE::Actor* attacker, const RE::Actor* victim, float range)
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

        if ((actor == attacker) || (actor == victim) || actor->IsDead() || actor->AsActorState()->IsBleedingOut() || actor->IsDisabled())
            continue;

        float dist = actor->GetPosition().GetDistance(attacker->GetPosition());
        if ((dist < range) && actor->IsHostileToActor(const_cast<RE::Actor*>(attacker)))
        {
            logger::debug("{} in range!", actor->GetName());
            return false;
        }
    }
    // EXTRA: CHECK PLAYER
    if (!attacker->IsPlayerRef() && !victim->IsPlayerRef())
        if (RE::Actor* player = RE::PlayerCharacter::GetSingleton(); player)
        {
            float dist = player->GetPosition().GetDistance(attacker->GetPosition());
            if ((dist < range) && const_cast<RE::Actor*>(attacker)->IsHostileToActor(player))
                return false;
        }

    return true;
}

RE::Actor* getNearestNPC(RE::Actor* origin, float max_range)
{
    logger::debug("getNearestNPC");
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

    float      min_dist  = max_range;
    RE::Actor* min_actor = nullptr;
    for (auto actor_handle : process_lists->highActorHandles)
    {
        if (!actor_handle || !actor_handle.get())
            continue;

        auto actor = actor_handle.get().get();

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

void playPairedIdle(RE::TESIdleForm* idle, RE::Actor* attacker, RE::Actor* victim)
{
    auto edid = idle->GetFormEditorID();
    logger::debug("Now playing {} between {} and {}", edid, attacker->GetName(), victim->GetName());
    _playPairedIdle(attacker->GetActorRuntimeData().currentProcess, attacker, RE::DEFAULT_OBJECT::kActionIdle, idle, true, false, victim);
    kaputt::setStatusMessage(fmt::format("Last played by this mod: {}", edid)); // notify menu
}
void testPlayPairedIdle(RE::TESIdleForm* idle, float max_range)
{
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player)
    {
        logger::info("No player found!");
        return;
    }
    auto victim = getNearestNPC(player, max_range);
    if (!victim)
    {
        logger::info("No target found!");
        return;
    }

    playPairedIdle(idle, player, victim);
}

std::string getSkeletonRace(const RE::Actor* actor)
{
    auto skel = actor->GetRace()->skeletonModels[actor->GetActorBase()->IsFemale()].model;
    // if (!stricmp(skel.c_str(), "Actors\\Character\\Character Assets\\skeleton.nif")) return "human";
    // if (!stricmp(skel.c_str(), "actors\\Character\\Character Assets Female\\skeleton_female.nif")) return "human";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\DwarvenBallistaCenturion\\Character Assets\\skeleton.nif")) return "ballista";
    if (!stricmp(skel.c_str(), "Actors\\Bear\\Character Assets\\skeleton.nif")) return "bear";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\BoarRiekling\\Character Assets\\SkeletonBoar.nif")) return "boar";
    if (!stricmp(skel.c_str(), "Actors\\DwarvenSteamCenturion\\Character Assets\\skeleton.nif")) return "centurion";
    if (!stricmp(skel.c_str(), "Actors\\DLC01\\ChaurusFlyer\\Character Assets\\skeleton.nif")) return "chaurushunter";
    if (!stricmp(skel.c_str(), "Actors\\Dragon\\Character Assets\\Skeleton.nif")) return "dragon";
    if (!stricmp(skel.c_str(), "Actors\\Draugr\\Character Assets\\Skeleton.nif")) return "draugr";
    if (!stricmp(skel.c_str(), "Actors\\Draugr\\Character Assets\\SkeletonF.nif")) return "draugr";
    if (!stricmp(skel.c_str(), "Actors\\Draugr\\Character Assets\\SkeletonS.nif")) return "skeleton";
    if (!stricmp(skel.c_str(), "Actors\\Falmer\\Character Assets\\Skeleton.nif")) return "falmer";
    if (!stricmp(skel.c_str(), "Actors\\DLC01\\VampireBrute\\Character Assets\\skeleton.nif")) return "gargoyle";
    if (!stricmp(skel.c_str(), "Actors\\Giant\\Character Assets\\skeleton.nif")) return "giant";
    if (!stricmp(skel.c_str(), "Actors\\Hagraven\\Character Assets\\skeleton.nif")) return "hagraven";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\BenthicLurker\\Character Assets\\skeleton.nif")) return "lurker";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\Riekling\\Character Assets\\skeleton.nif")) return "riekling";
    if (!stricmp(skel.c_str(), "Actors\\SabreCat\\Character Assets\\Skeleton.nif")) return "sabrecat";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\Scrib\\Character Assets\\skeleton.nif")) return "ashhopper";
    if (!stricmp(skel.c_str(), "Actors\\FrostbiteSpider\\Character Assets\\skeleton.nif")) return "spider";
    if (!stricmp(skel.c_str(), "Actors\\Spriggan\\Character Assets\\skeleton.nif")) return "spriggan";
    if (!stricmp(skel.c_str(), "Actors\\Troll\\Character Assets\\skeleton.nif")) return "troll";
    if (!stricmp(skel.c_str(), "Actors\\Canine\\Character Assets Wolf\\skeleton.nif")) return "wolf";
    if (!stricmp(skel.c_str(), "Actors\\WerewolfBeast\\Character Assets\\skeleton.nif")) return "werewolf";
    if (!stricmp(skel.c_str(), "Actors\\VampireLord\\Character Assets\\Skeleton.nif")) return "vamplord";
    if (!stricmp(skel.c_str(), "Actors\\Chaurus\\Character Assets\\skeleton.nif")) return "chaurus";
    if (!stricmp(skel.c_str(), "Actors\\Deer\\Character Assets\\Skeleton.nif")) return "deer";
    if (!stricmp(skel.c_str(), "Actors\\Canine\\Character Assets Dog\\skeleton.nif")) return "dog";
    if (!stricmp(skel.c_str(), "Actors\\DragonPriest\\Character Assets\\skeleton.nif")) return "priest";
    if (!stricmp(skel.c_str(), "Actors\\DwarvenSphereCenturion\\Character Assets\\skeleton.nif")) return "sphere";
    if (!stricmp(skel.c_str(), "Actors\\DwarvenSpider\\Character Assets\\skeleton.nif")) return "dwarvenspider";
    if (!stricmp(skel.c_str(), "Actors\\AtronachFlame\\Character Assets\\skeleton.nif")) return "flameatronach";
    if (!stricmp(skel.c_str(), "Actors\\AtronachFrost\\Character Assets\\skeleton.nif")) return "frostatronach";
    if (!stricmp(skel.c_str(), "Actors\\AtronachStorm\\Character Assets\\skeleton.nif")) return "stormatronach";
    if (!stricmp(skel.c_str(), "Actors\\Goat\\Character Assets\\skeleton.nif")) return "goat";
    if (!stricmp(skel.c_str(), "Actors\\Horker\\Character Assets\\skeleton.nif")) return "horker";
    if (!stricmp(skel.c_str(), "Actors\\Horse\\Character Assets\\skeleton.nif")) return "horse";
    if (!stricmp(skel.c_str(), "Actors\\IceWraith\\Character Assets\\skeleton.nif")) return "wraith";
    if (!stricmp(skel.c_str(), "Actors\\Mammoth\\Character Assets\\skeleton.nif")) return "mammoth";
    if (!stricmp(skel.c_str(), "Actors\\Skeever\\Character Assets\\skeleton.nif")) return "skeever";
    if (!stricmp(skel.c_str(), "Actors\\Slaughterfish\\Character Assets\\skeleton.nif")) return "slaughterfish";
    if (!stricmp(skel.c_str(), "Actors\\Wisp\\Character Assets\\skeleton.nif")) return "wisp";
    if (!stricmp(skel.c_str(), "Actors\\Witchlight\\Character Assets\\skeleton.nif")) return "witchlight";
    if (!stricmp(skel.c_str(), "Actors\\Cow\\Character Assets\\skeleton.nif")) return "cow";
    if (!stricmp(skel.c_str(), "Actors\\Ambient\\Hare\\Character Assets\\skeleton.nif")) return "rabbit";
    if (!stricmp(skel.c_str(), "Actors\\Mudcrab\\Character Assets\\skeleton.nif")) return "mudcrab";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\HMDaedra\\Character Assets\\Skeleton.nif")) return "seeker";
    if (!stricmp(skel.c_str(), "Actors\\DLC02\\Netch\\CharacterAssets\\skeleton.nif")) return "netch";
    return {};
}

} // namespace kaputt