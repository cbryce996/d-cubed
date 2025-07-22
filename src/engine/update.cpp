#include "update.h"
#include "tasks.h"

UpdateManager::UpdateManager(const float interval_ms, std::function<void()> task)
    : interval(interval_ms), task(std::move(task)) {}

void UpdateManager::update(const float delta_time_ms, TaskScheduler& scheduler) {
    accumulated += delta_time_ms;
    if (accumulated >= interval) {
        scheduler.submit([this]() {
            task();
        });
        accumulated = 0.0f;
    }
}
