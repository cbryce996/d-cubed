#ifndef CLOCK_H
#define CLOCK_H

#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <chrono>

class Clock {
	using clock = std::chrono::high_resolution_clock;

  public:
	float avg_fps = 0.0f;
	float fixed_dt_ms;

	explicit Clock (int target_fps, float fixed_dt_ms = 16.6667f)
		: fixed_dt_ms (fixed_dt_ms), target_fps (target_fps),
		  frame_delay_ms (1000.0f / target_fps) {
		last_frame_start = clock::now ();
		last_stat_time = last_frame_start;
	}

	float begin_frame () {
		frame_start = clock::now ();

		float dt = std::chrono::duration<float, std::milli> (
					   frame_start - last_frame_start
		)
					   .count ();

		last_frame_start = frame_start;

		dt = std::min (dt, max_frame_dt_ms);

		accumulator_ms += dt;
		return dt;
	}

	void end_frame () {
		auto frame_end = clock::now ();

		float frame_time_ms = std::chrono::duration<float, std::milli> (
								  frame_end - frame_start
		)
								  .count ();

		if (frame_time_ms < frame_delay_ms) {
			auto next_frame_time = frame_start
								   + std::chrono::microseconds (
									   (int)(frame_delay_ms * 1000)
								   );
			std::this_thread::sleep_until (next_frame_time);
			frame_end = clock::now ();
			frame_time_ms = std::chrono::duration<float, std::milli> (
								frame_end - frame_start
			)
								.count ();
		}

		accumulated_ms += frame_time_ms;
		frame_count++;
	}

	bool should_log_stats () {
		auto now = clock::now ();
		if (std::chrono::duration_cast<std::chrono::seconds> (
				now - last_stat_time
			)
				.count ()
			>= 1) {

			avg_fps = 1000.0f / (accumulated_ms / frame_count);
			accumulated_ms = 0.0f;
			frame_count = 0;
			last_stat_time = now;
			return true;
		}
		return false;
	}

	bool should_step_simulation () const {
		return accumulator_ms >= fixed_dt_ms;
	}

	void consume_simulation_step () { accumulator_ms -= fixed_dt_ms; }

	float alpha () const { return accumulator_ms / fixed_dt_ms; }

  private:
	int target_fps;
	float frame_delay_ms;
	const float max_frame_dt_ms = 250.0f;

	clock::time_point frame_start;
	clock::time_point last_frame_start;
	clock::time_point last_stat_time;

	float accumulator_ms = 0.0f;

	float accumulated_ms = 0.0f;
	int frame_count = 0;
};

#endif // CLOCK_H
