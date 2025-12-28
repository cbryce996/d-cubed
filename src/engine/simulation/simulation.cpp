#include "simulation.h"

#include "game/core/items/types.h"

Simulation::Simulation () {}

void Simulation::update (float delta_time_ms) {
	simulation_time_ms += delta_time_ms;
	for (Schedule& schedule : schedules) {
		schedule.update (delta_time_ms, task_scheduler);
	}
}