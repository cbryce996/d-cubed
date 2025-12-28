#ifndef GAME_H
#define GAME_H

#include "engine/engine.h"

#include <player.h>
#include <update.h>
#include <vector>

class GameManager {
  public:
	float delta_time_ms = 0.0f;
	std::vector<UpdateManager> update_managers;
	Player player;

	std::mutex mutex;

	explicit GameManager (std::unique_ptr<Engine> engine);
	~GameManager ();

	void run ();
	void update (
		float delta_time, TaskScheduler& scheduler, const InputManager& input
	);
	static void handle_input (const InputManager& input);
	void
	write_render_state (float elapsed_time, RenderState* render_state) const;

  private:
	std::unique_ptr<Engine> engine;

	bool running = false;

	float decay_timer = 0.0f;
	static constexpr float DECAY_INTERVAL_MS = 3000.0f;

	float crafting_timer = 0.0f;
	static constexpr float CRAFTING_INTERVAL_MS = 1000.0f;

	static void calculate_item_decays (float delta_time_ms);
	static void calculate_item_crafting_progress (float delta_time_ms);
};

#endif // GAME_H
