#include <gtest/gtest.h>

#include "../src/engine/tasks/tasks.h"

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
	int hardware_threads = std::thread::hardware_concurrency ();
	ASSERT_EQ (hardware_threads, task_scheduler->thread_count);
}

TEST_F (TasksTest, JoinsThreadsOnStop) {
	TaskScheduler* task_scheduler = new TaskScheduler ();

	task_scheduler->submit (task);
	task_scheduler->start ();

	std::this_thread::sleep_for (std::chrono::milliseconds (10));
	ASSERT_TRUE (task_scheduler->running);

	task_scheduler->stop ();
	ASSERT_FALSE (task_scheduler->running);
	ASSERT_TRUE (task_ran);
}
