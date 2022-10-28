#include <spdlog/sinks/basic_file_sink.h>

#include "kaputt.h"
#include "menu.h"
#include "cathub.h"
#include "re.h"

#define DBGMSG

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

            logger::info("Installing hook");
            // stl::write_thunk_call<ProcessHitHook>();

            if (!Kaputt::getSingleton()->init())
                setStatusMessage("Something went wrong during Kaputt initialization. Plugin is not disabled but you may check the log.");

            integrateCatHub(); // Cathub
            break;
        default:
            break;
    }
}
} // namespace kaputt

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    using namespace kaputt;

    installLog();

    auto* plugin  = SKSE::PluginDeclaration::GetSingleton();
    auto  version = plugin->GetVersion();
    logger::info("{} {} is loading...", plugin->GetName(), version);

    SKSE::Init(skse);
    SKSE::AllocTrampoline(14 * 2);

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", processMessage))
        return false;

    logger::info("{} has finished loading.", plugin->GetName());
    return true;
}