#ifndef SYSTEM_TIMER_H
#define SYSTEM_TIMER_H

#include <functional>

class TaskScheduler;

class UpdateManager {
public:
    UpdateManager(float interval_ms, std::function<void(float)> task);
    void update(float delta_time_ms, TaskScheduler& scheduler);

private:
    float interval;
    float accumulated = 0.0f;
    std::function<void(float)> task;
};

#endif // SYSTEM_TIMER_H
