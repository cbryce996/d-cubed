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
	explicit TaskScheduler(
		size_t num_threads = std::thread::hardware_concurrency()
	);
	~TaskScheduler();

	void submit(std::function<void()> job);
	void start();
	void stop();

   private:
	void do_work();

	std::vector<std::thread> workers;
	std::queue<std::function<void()>> task_queue;
	std::mutex queue_mutex;
	std::condition_variable cv;
	std::atomic<bool> running = false;
	size_t thread_count = 0;
};

#endif	// TASKS_H