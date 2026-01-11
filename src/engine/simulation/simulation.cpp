#include "simulation.h"

Simulation::Simulation () = default;

void Simulation::update (const float delta_time_ms) {
	simulation_time_ms += delta_time_ms;
	for (Schedule& schedule : schedules) {
		schedule.update (delta_time_ms, task_scheduler);
	}
}