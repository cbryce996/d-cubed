#include "../game/update.h"

#include "../engine/tasks/tasks.h"

UpdateManager::UpdateManager (
	float interval_ms, std::function<void (float)> task
)
	: interval (interval_ms), task (std::move (task)) {}

void UpdateManager::update (float delta_time_ms, TaskScheduler& scheduler) {
	accumulated += delta_time_ms;
	if (accumulated >= interval) {
		scheduler.submit ([delta_time_ms, task = this->task] () {
			task (delta_time_ms);
		});
		accumulated = 0.0f;
	}
}
