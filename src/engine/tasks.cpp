#include "tasks.h"

#include <iostream>

TaskScheduler::TaskScheduler(const size_t num_threads)
	: thread_count(num_threads),
	  running(false) {}

TaskScheduler::~TaskScheduler() {
	stop();
}

void TaskScheduler::start() {
	if (running)
		return;

	running = true;
	workers.reserve(thread_count);
	for (size_t i = 0; i < thread_count; ++i) {
		workers.emplace_back(&TaskScheduler::do_work, this);
	}
}

void TaskScheduler::stop() {
	if (!running)
		return;

	running = false;
	cv.notify_all();

	for (auto& t : workers) {
		if (t.joinable()) {
			t.join();
		}
	}

	workers.clear();
}

void TaskScheduler::submit(std::function<void()> job) {
	{
		std::lock_guard lock(queue_mutex);
		task_queue.push(std::move(job));
	}
	cv.notify_one();
}

void TaskScheduler::do_work() {
	while (running) {
		std::function<void()> task;
		{
			;
			std::unique_lock lock(queue_mutex);
			cv.wait(lock, [this] { return !task_queue.empty() || !running; });

			if (!running && task_queue.empty())
				return;

			task = std::move(task_queue.front());
			task_queue.pop();
		}

		std::cout << "[Worker] Executing job on thread " << std::this_thread::get_id() << "\n";
		task();
	}
}
