#include "simulation/tasks/tasks.h"

#include <future>
#include <gtest/gtest.h>
#include <thread>

class TasksTest : public ::testing::Test {
  protected:
	void SetUp () override {
		task = [&] () {
			task_ran = true;
			std::this_thread::sleep_for (std::chrono::milliseconds (50));
		};
	}
	std::atomic<bool> task_ran{false};
	std::function<void ()> task;
};

TEST_F (TasksTest, SetsThreadCountOnInit) {
	TaskScheduler* task_scheduler = new TaskScheduler ();
	int hardware_threads = std::min<size_t> (
		4, std::max<size_t> (1, std::thread::hardware_concurrency () - 1)
	);
	ASSERT_EQ (hardware_threads, task_scheduler->thread_count);
}

TEST_F (TasksTest, JoinsThreadsOnStop) {
	TaskScheduler task_scheduler;

	std::promise<void> ran;
	auto future = ran.get_future ();

	task_scheduler.submit ([&] {
		task_ran = true;
		ran.set_value ();
	});

	task_scheduler.start ();
	ASSERT_TRUE (task_scheduler.running);

	ASSERT_EQ (
		future.wait_for (std::chrono::seconds (1)), std::future_status::ready
	);

	ASSERT_TRUE (task_ran);

	task_scheduler.stop ();
	ASSERT_FALSE (task_scheduler.running);
}
