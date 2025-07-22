#ifndef GAME_H
#define GAME_H

#include "../engine/inputs.h"
#include "../engine/tasks.h"
#include "player.h"
#include "render/render.h"

class UpdateManager;

class GameManager {
   public:
	float delta_time_ms = 0.0f;
	std::vector<UpdateManager> update_managers;
	Player player;

	GameManager();
	~GameManager();

	void update(
		float delta_time,
		TaskScheduler& scheduler,
		const InputManager& input
	);
	void write_render_state(RenderState& render_state) const;

   private:
	float decay_timer = 0.0f;
	static constexpr float DECAY_INTERVAL_MS = 3000.0f;

	float crafting_timer = 0.0f;
	static constexpr float CRAFTING_INTERVAL_MS = 1000.0f;

	static void calculate_item_decays();
	void calculate_item_crafting_progress() const;
};

#endif	// GAME_H