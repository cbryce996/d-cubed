#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <functional>

class TaskScheduler;

class Schedule {
  public:
	Schedule (float interval_ms, std::function<void (float)> task);
	void update (float delta_time_ms, TaskScheduler& scheduler);

  private:
	float interval;
	float accumulated = 0.0f;
	std::function<void (float)> task;
};

#endif // SCHEDULE_H
