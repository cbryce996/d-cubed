#ifndef SIMULATION_H
#define SIMULATION_H

#include "engine/tasks/schedule.h"
#include "engine/tasks/tasks.h"

class Simulation {
  public:
	Simulation ();
	void update (float delta_time_ms);

	TaskScheduler task_scheduler;

	std::vector<Schedule> schedules;
	float simulation_time_ms = 0.0f;
};

#endif // SIMULATION_H
