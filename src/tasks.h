#pragma once

namespace kaputt
{
class TaskManager
{
public:
    static TaskManager* getSingleton()
    {
        static TaskManager module;
        return std::addressof(module);
    }

    inline void addTask(double countdown, std::function<void()> func)
    {
        funcs_mutex.lock();
        funcs.push_back(std::make_pair(countdown, func));
        funcs_mutex.unlock();
    }
    void update();
    void flush();

private:
    std::vector<std::pair<double, std::function<void()>>> funcs;
    std::mutex                                            funcs_mutex;
};
} // namespace kaputt
