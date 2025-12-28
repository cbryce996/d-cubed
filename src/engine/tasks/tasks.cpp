#include "tasks.h"

#include <iostream>

TaskScheduler::TaskScheduler (const size_t num_threads)
	: thread_count (num_threads) {}

TaskScheduler::~TaskScheduler () { stop (); }

void TaskScheduler::start () {
	if (running)
		return;

	running = true;
	workers.reserve (thread_count);
	for (size_t i = 0; i < thread_count; ++i) {
		workers.emplace_back (&TaskScheduler::do_work, this);
		pthread_set_qos_class_self_np (QOS_CLASS_USER_INTERACTIVE, 0);
	}
}

void TaskScheduler::stop () {
	if (!running)
		return;

	running = false;
	cv.notify_all ();

	for (auto& t : workers)
		if (t.joinable ())
			t.join ();

	workers.clear ();
}

void TaskScheduler::submit (std::function<void ()> job) {
	{
		std::lock_guard lock (queue_mutex);
		if (!running)
			return;
		task_queue.push (std::move (job));
	}
	cv.notify_one ();
}

void TaskScheduler::do_work () {
	while (true) {
		std::function<void ()> task;
		{
			std::unique_lock<std::mutex> lock (queue_mutex);
			cv.wait (lock, [this] { return !running || !task_queue.empty (); });

			if (!running && task_queue.empty ())
				return;

			task = std::move (task_queue.front ());
			task_queue.pop ();
		}

		if (task) {
			task ();
		}
	}
}