#include "simulation.h"

#include "game/core/items/types.h"

Simulation::Simulation () {}

void Simulation::update (const float delta_time) {
	delta_time_ms = delta_time;

	for (Schedule& schedule : schedules)
		schedule.update (delta_time, task_scheduler);
}