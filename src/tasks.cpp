#include "tasks.h"

namespace kaputt
{

void TaskManager::update()
{
    auto delta_time = *REL::Relocation<float*>{RELOCATION_ID(523661, 410200)};
    funcs_mutex.lock();
    std::erase_if(
        funcs, [=](auto& pair) {
            auto& [delay_time, func] = pair;
            delay_time -= delta_time;
            if (delay_time < 0)
            {
                logger::debug("Executing delayed func");
                func();
                return true;
            }
            return false;
        });
    funcs_mutex.unlock();
}

void TaskManager::flush()
{
    funcs_mutex.lock();
    funcs.clear();
    funcs_mutex.unlock();
}
} // namespace kaputt