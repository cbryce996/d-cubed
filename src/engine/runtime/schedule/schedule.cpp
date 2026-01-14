#include "schedule.h"

Schedule::Schedule (float interval_ms, std::function<void (float)> task)
	: interval (interval_ms), task (std::move (task)) {}

void Schedule::update (float delta_time_ms, TaskScheduler& scheduler) {
	accumulated += delta_time_ms;
	if (accumulated >= interval) {
		float dt = accumulated;
		scheduler.submit ([task = this->task, dt] () { task (dt); });
		accumulated = 0.0f;
	}
}
