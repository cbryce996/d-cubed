#include "render/graph/graph.h"
#include "render/render.h"
#include <gtest/gtest.h>

#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

struct FakeContext : RenderContext {};

class RenderGraphTest : public ::testing::Test {
  protected:
	RenderGraph graph;
	FakeContext ctx;
	std::vector<std::string> execution_log;

	void SetUp () override { execution_log.clear (); }

	RenderPassInstance
	MakePass (const std::string& name, std::vector<std::string> deps = {}) {
		RenderPassInstance pass{};
		pass.name = name;
		pass.type = RenderPassType::Geometry;
		pass.dependencies = std::move (deps);
		pass.execute = [] (RenderContext&, RenderPassInstance&) {};
		return pass;
	}

	RenderPassInstance MakeRecordingPass (
		const std::string& name, std::vector<std::string> deps = {}
	) {
		RenderPassInstance pass{};
		pass.name = name;
		pass.type = RenderPassType::Geometry;
		pass.dependencies = std::move (deps);
		pass.execute = [this, name] (RenderContext&, RenderPassInstance&) {
			execution_log.push_back (name);
		};
		return pass;
	}
};

TEST_F (RenderGraphTest, AddAndGetPass) {
	auto pass = MakePass ("geometry");
	graph.add_pass (pass);

	auto* fetched = graph.get_render_pass ("geometry");

	ASSERT_NE (fetched, nullptr);
	EXPECT_EQ (fetched->name, "geometry");
}

TEST_F (RenderGraphTest, ReturnsNullForMissingPass) {
	auto* fetched = graph.get_render_pass ("missing");
	EXPECT_EQ (fetched, nullptr);
}

TEST_F (RenderGraphTest, ExecutesSinglePass) {
	bool executed = false;

	RenderPassInstance pass{};
	pass.name = "single";
	pass.type = RenderPassType::Geometry;
	pass.execute = [&] (RenderContext&, RenderPassInstance&) {
		executed = true;
	};

	graph.add_pass (pass);
	graph.execute_all (ctx);

	EXPECT_TRUE (executed);
}

TEST_F (RenderGraphTest, RespectsDependencyOrder) {
	graph.add_pass (MakeRecordingPass ("A"));
	graph.add_pass (MakeRecordingPass ("B", {"A"}));
	graph.add_pass (MakeRecordingPass ("C", {"B"}));

	graph.execute_all (ctx);

	const std::vector<std::string> expected = {"A", "B", "C"};
	EXPECT_EQ (execution_log, expected);
}

TEST_F (RenderGraphTest, MultipleDependenciesOrder) {
	graph.add_pass (MakeRecordingPass ("A"));
	graph.add_pass (MakeRecordingPass ("B"));
	graph.add_pass (MakeRecordingPass ("C", {"A", "B"}));

	graph.execute_all (ctx);

	const auto posA = std::ranges::find (execution_log, "A");
	const auto posB = std::ranges::find (execution_log, "B");
	const auto posC = std::ranges::find (execution_log, "C");

	ASSERT_NE (posA, execution_log.end ());
	ASSERT_NE (posB, execution_log.end ());
	ASSERT_NE (posC, execution_log.end ());

	EXPECT_LT (posA, posC);
	EXPECT_LT (posB, posC);
}

TEST_F (RenderGraphTest, DetectsMissingDependency) {
	graph.add_pass (MakePass ("A", {"B"}));
	graph.add_pass (MakePass ("B", {"A"}));

	auto* A = graph.get_render_pass ("A");
	auto* B = graph.get_render_pass ("B");

	EXPECT_EQ (A, nullptr);
	EXPECT_EQ (B, nullptr);
}

TEST_F (RenderGraphTest, DetectsCycle) {
	graph.add_pass (MakePass ("A"));
	graph.add_pass (MakePass ("B", {"A"}));
	graph.add_pass (MakePass ("A", {"B"}));

	auto* A = graph.get_render_pass ("A");
	auto* B = graph.get_render_pass ("B");
	auto* C = graph.get_render_pass ("C");

	ASSERT_NE (A, nullptr);
	ASSERT_NE (B, nullptr);
	ASSERT_EQ (C, nullptr);
}

TEST_F (RenderGraphTest, CompletionFlagIsSet) {
	graph.add_pass (MakePass ("A"));
	graph.add_pass (MakePass ("B", {"A"}));

	graph.execute_all (ctx);

	auto* A = graph.get_render_pass ("A");
	auto* B = graph.get_render_pass ("B");

	ASSERT_NE (A, nullptr);
	ASSERT_NE (B, nullptr);

	EXPECT_TRUE (A->completed);
	EXPECT_TRUE (B->completed);
}

TEST_F (RenderGraphTest, DeterministicExecution) {
	graph.add_pass (MakeRecordingPass ("A"));
	graph.add_pass (MakeRecordingPass ("B", {"A"}));
	graph.add_pass (MakeRecordingPass ("C", {"B"}));

	graph.execute_all (ctx);

	const std::vector<std::string> first = execution_log;

	RenderGraph graph2;
	std::vector<std::string> second_log;

	auto make2 =
		[&] (const std::string& name, std::vector<std::string> deps = {}) {
			RenderPassInstance pass{};
			pass.name = name;
			pass.type = RenderPassType::Geometry;
			pass.dependencies = std::move (deps);
			pass.execute = [&] (RenderContext&, RenderPassInstance&) {
				second_log.push_back (name);
			};
			return pass;
		};

	graph2.add_pass (make2 ("A"));
	graph2.add_pass (make2 ("B", {"A"}));
	graph2.add_pass (make2 ("C", {"B"}));

	graph2.execute_all (ctx);

	EXPECT_EQ (first, second_log);
}
