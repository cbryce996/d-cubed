#ifndef SIMULATION_H
#define SIMULATION_H

#include "engine/tasks/schedule.h"
#include "engine/tasks/tasks.h"
#include "game/entity.h"
#include "game/player.h"

class Simulation {
  public:
	Simulation ();
	void update (float delta_time_ms);

	TaskScheduler task_scheduler;

	std::vector<Schedule> schedules;
	float delta_time_ms = 0.0f;
	Player player;

  private:
	std::vector<Entity> entities;
};

#endif // SIMULATION_H
