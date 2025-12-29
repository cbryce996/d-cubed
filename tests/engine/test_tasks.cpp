#include <gtest/gtest.h>

#include "../../src/engine/tasks/tasks.h"

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
	TaskScheduler* task_scheduler = new TaskScheduler ();

	task_scheduler->submit (task);
	task_scheduler->start ();
	ASSERT_TRUE (task_scheduler->running);

	task_scheduler->wait_idle ();
	ASSERT_TRUE (task_ran);

	task_scheduler->stop ();
	ASSERT_FALSE (task_scheduler->running);
}
