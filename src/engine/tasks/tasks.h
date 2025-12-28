#ifndef TASKS_H
#define TASKS_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class TaskScheduler {
  public:
	explicit TaskScheduler (
		size_t num_threads = std::min<size_t> (
			4, std::max<size_t> (1, std::thread::hardware_concurrency () - 1)
		)
	);
	~TaskScheduler ();

	void submit (std::function<void ()> job);
	void start ();
	void stop ();
	void wait_idle ();

	size_t thread_count = 0;
	std::atomic<bool> running = false;

	std::atomic<int> busy_tasks{0};
	std::condition_variable idle_condition_variable;
	std::mutex idle_mutex;

  private:
	void do_work ();

	std::vector<std::thread> workers;
	std::queue<std::function<void ()>> task_queue;
	std::mutex queue_mutex;
	std::condition_variable condition_variable;
};

#endif // TASKS_H
