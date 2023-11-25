#include <spdlog/sinks/basic_file_sink.h>

#include "kaputt.h"
#include "menu.h"
#include "cathub.h"
#include "re.h"
#include "tasks.h"
#include "PrecisionAPI.h"
#include "trigger.h"

#define DLLEXPORT __declspec(dllexport)

// #define DBGMSG

namespace kaputt
{
bool installLog()
{
    auto path = logger::log_directory();
    if (!path)
        return false;

    *path /= fmt::format(FMT_STRING("{}.log"), SKSE::PluginDeclaration::GetSingleton()->GetName());
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef DBGMSG
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
#else
    log->set_level(spdlog::level::trace);
    log->flush_on(spdlog::level::trace);
#endif

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%H:%M:%S:%e][%5l] %v"s);

    return true;
}

void initPrecisionAPI()
{
    auto result = reinterpret_cast<PRECISION_API::IVPrecision1*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V1));
    if (result)
    {
        logger::info("Obtained PrecisionAPI");
        auto res = result->AddPreHitCallback(
            SKSE::GetPluginHandle(),
            [](const PRECISION_API::PrecisionHitData& precision_hitdata) -> PRECISION_API::PreHitCallbackReturn {
                auto attacker = precision_hitdata.attacker;
                if (!attacker)
                    return {};
                auto target = precision_hitdata.target ? precision_hitdata.target->As<RE::Actor>() : nullptr;
                if (!target)
                    return {};
                auto attack_weap = attacker->GetAttackingWeapon();
                if (!attack_weap)
                    return {};
                RE::HitData hitdata;
                _hitdataCtor(&hitdata);
                hitdata.Populate(attacker, target, attack_weap);

                // if (PostHitTrigger::getSingleton()->process(target, hitdata))
                // {
                //     logger::debug("Sent!");
                //     return {.bIgnoreHit = true};
                // }
                // return {};
                return {.bIgnoreHit = true};
            });
        if (res == PRECISION_API::APIResult::OK || res == PRECISION_API::APIResult::AlreadyRegistered)
            logger::info("Collision prehit callback successfully registered.");
    }
    else
        logger::info("Precision API not found.");
}

void integrateCatHub()
{
    logger::info("Looking for CatHub...");

    auto result = cathub::RequestCatHubAPI();
    if (result.index() == 0)
    {
        auto cathub_api = std::get<0>(result);
        ImGui::SetCurrentContext(cathub_api->getContext());
        cathub_api->addMenu("Kaputt", drawCatMenu);
        logger::info("CatHub integration succeed!");
    }
    else
        logger::warn("CatHub integration failed! In-game config disabled. Error: {}", std::get<1>(result));
}

void processMessage(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type)
    {
        case SKSE::MessagingInterface::kDataLoaded:
            logger::info("Game: data loaded");

            if (!Kaputt::getSingleton()->init())
                setStatusMessage("Something wrong while initializing kaputt. Please check the log.");

            if (Kaputt::getSingleton()->isReady())
            {
                integrateCatHub(); // Cathub
                // initPrecisionAPI();

                logger::info("Installing hook...");
                stl::write_thunk_call<ProcessHitHook>();
                stl::write_thunk_call<AttackActionHook>();
                // stl::write_thunk_call<UpdateHook>();

                logger::info("Registering event sinks...");
                InputEventSink::RegisterSink();
            }

            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            logger::debug("Game: save loaded");

            if (Kaputt::getSingleton()->isReady())
            {
                TaskManager::getSingleton()->flush();
                PlayerAnimGraphEventSink::RegisterSink();
                Kaputt::getSingleton()->applyRefs();
            }

            break;
        default:
            break;
    }
}
} // namespace kaputt

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    using namespace kaputt;

#ifndef NDEBUG
    while (!WinAPI::IsDebuggerPresent())
    {};
#endif
    installLog();
    logger::info("Loaded {} {}", Plugin::NAME, Plugin::VERSION.string());

    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(14 * 3);

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", processMessage))
        return false;

    logger::info("{} has finished loading.", Plugin::NAME);
    return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
    SKSE::PluginVersionData v;
    v.PluginName(Plugin::NAME.data());
    v.PluginVersion(Plugin::VERSION);
    v.UsesAddressLibrary(true);
    v.HasNoStructUse();
    return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
    pluginInfo->name        = SKSEPlugin_Version.pluginName;
    pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
    pluginInfo->version     = SKSEPlugin_Version.pluginVersion;
    return true;
}
