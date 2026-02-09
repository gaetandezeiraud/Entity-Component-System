/*
MIT License

Copyright (c) 2026 Gaëtan Dezeiraud

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gtest/gtest.h>
#include <chrono>
#include <numeric>
#include <set>
#include <memory>
#include <algorithm>
#include <random>

#include "../src/ECS/ECS.h"
#include "../src/PrimitiveTypes.h"

// Dummy component for testing
struct TestComponent
{
	int value;
	TestComponent(int v = 0) : value(v) {}
};

TEST(ECSTest, EntityComponentLifecycle) {
	using namespace ECS;
	Registry registry;

	// Create entity and add component
	auto entity = registry.CreateEntity();
	registry.AddComponent<TestComponent>(entity, 42);

	// Check component exists and value is correct
	EXPECT_TRUE(registry.HasComponent<TestComponent>(entity));
	EXPECT_EQ(registry.GetComponent<TestComponent>(entity).value, 42);

	// Remove component and check
	registry.RemoveComponent<TestComponent>(entity);
	EXPECT_FALSE(registry.HasComponent<TestComponent>(entity));
}

TEST(ECSTest, EntityMemberAddComponent) {
	using namespace ECS;
	Registry registry;

	auto entity = registry.CreateEntity();
	// Use Entity's member AddComponent wrapper
	entity.AddComponent<TestComponent>(55);

	EXPECT_TRUE(entity.HasComponent<TestComponent>());
	EXPECT_EQ(entity.GetComponent<TestComponent>().value, 55);
}

TEST(ECSTest, EntityTagAndGroup) {
	using namespace ECS;
	Registry registry;
	auto entity = registry.CreateEntity();

	registry.TagEntity(entity, "player");
	EXPECT_TRUE(registry.EntityHasTag(entity, "player"));
	EXPECT_EQ(registry.GetEntityByTag("player"), entity);

	registry.GroupEntity(entity, "enemies");
	EXPECT_TRUE(registry.EntityBelongsToGroup(entity, "enemies"));
	auto groupEntities = registry.GetEntitiesByGroup("enemies");
	EXPECT_EQ(groupEntities.size(), 1);
	EXPECT_EQ(groupEntities[0], entity);
}

TEST(ECSTest, StressTest_MassEntityCreation) {
	using namespace ECS;
	Registry registry;
	constexpr size_t N = 100000;
	std::vector<Entity> entities;
	entities.reserve(N);

	// Create many entities and add components
	for (size_t i = 0; i < N; ++i) {
		auto e = registry.CreateEntity();
		registry.AddComponent<TestComponent>(e, static_cast<int>(i));
		entities.push_back(e);
	}

	// Check all entities have the component
	for (size_t i = 0; i < N; ++i) {
		EXPECT_TRUE(registry.HasComponent<TestComponent>(entities[i]));
		EXPECT_EQ(registry.GetComponent<TestComponent>(entities[i]).value, i);
	}

	// Remove half and check
	for (size_t i = 0; i < N; i += 2) {
		registry.RemoveComponent<TestComponent>(entities[i]);
		EXPECT_FALSE(registry.HasComponent<TestComponent>(entities[i]));
	}
}

// Suggestion: Add a performance test for component access
TEST(ECSTest, ComponentAccessPerformance) {
	using namespace ECS;
	Registry registry;
	constexpr size_t N = 100000;
	std::vector<Entity> entities;
	entities.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		auto e = registry.CreateEntity();
		registry.AddComponent<TestComponent>(e, static_cast<int>(i));
		entities.push_back(e);
	}

	// Time component access
	auto start = std::chrono::high_resolution_clock::now();
	int sum = 0;
	for (size_t i = 0; i < N; ++i) {
		sum += registry.GetComponent<TestComponent>(entities[i]).value;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	int expected_sum = 0;
	for (size_t i = 0; i < N; ++i) {
		expected_sum += static_cast<int>(i);
	}
	EXPECT_EQ(sum, expected_sum);

	// Print timing for informational purposes
	std::cout << "[          ] Component access time: " << ms << " ms\n";
}

TEST(ECSTest, ComponentAccessPerformance_Random) {
	using namespace ECS;
	Registry registry;
	constexpr size_t N = 100000;
	std::vector<Entity> entities;
	entities.reserve(N);

	for (size_t i = 0; i < N; ++i) {
		auto e = registry.CreateEntity();
		registry.AddComponent<TestComponent>(e, static_cast<int>(i));
		entities.push_back(e);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(entities.begin(), entities.end(), g);

	auto start = std::chrono::high_resolution_clock::now();

	long long sum = 0;
	for (size_t i = 0; i < N; ++i) {
		sum += registry.GetComponent<TestComponent>(entities[i]).value;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	EXPECT_GT(sum, 0);

	std::cout << "[          ] Random access time (100k): " << ms << " ms\n";
}

// Test: Entity Reuse and ID Recycling
TEST(ECSTest, EntityReuseAndIDRecycling) {
	using namespace ECS;
	Registry registry;
	constexpr size_t N = 1000;
	std::vector<Entity> entities;
	entities.reserve(N);

	// Create N entities
	for (size_t i = 0; i < N; ++i) {
		entities.push_back(registry.CreateEntity());
	}

	// Store original INDICES (not full IDs)
	std::set<u32> originalIndices;
	std::set<EntityID> originalFullIds;
	for (const auto& e : entities) {
		originalIndices.insert(GetEntityIndex(e.GetId()));
		originalFullIds.insert(e.GetId());
	}

	// Kill all entities
	for (auto& e : entities) {
		registry.KillEntity(e);
	}
	registry.Update();

	// Create N new entities
	std::vector<Entity> newEntities;
	newEntities.reserve(N);
	for (size_t i = 0; i < N; ++i) {
		newEntities.push_back(registry.CreateEntity());
	}

	// Verify Recycling
	std::set<u32> recycledIndices;
	std::set<EntityID> recycledFullIds;
	for (const auto& e : newEntities) {
		recycledIndices.insert(GetEntityIndex(e.GetId()));
		recycledFullIds.insert(e.GetId());
	}

	// CHECK 1: INDICES SHOULD BE REUSED (Memory efficiency)
	size_t indexOverlap = 0;
	for (auto index : originalIndices) {
		if (recycledIndices.count(index)) ++indexOverlap;
	}
	EXPECT_GT(indexOverlap, 0) << "Entity memory slots (indices) were not recycled!";
	EXPECT_EQ(indexOverlap, N) << "All indices should have been recycled given the exact same count.";

	// CHECK 2: FULL IDS SHOULD BE DIFFERENT (Safety / Versioning)
	size_t idOverlap = 0;
	for (auto id : originalFullIds) {
		if (recycledFullIds.count(id)) ++idOverlap;
	}
	EXPECT_EQ(idOverlap, 0) << "CRITICAL: Full Entity IDs were recycled! Versioning is not working.";
}

// Test: Component Type Stress
TEST(ECSTest, ComponentTypeStress) {
	using namespace ECS;
	Registry registry;
	auto entity = registry.CreateEntity();

	// Define many dummy component types
	struct CompA { int v = 1; };
	struct CompB { float v = 2.0f; };
	struct CompC { double v = 3.0; };
	struct CompD { char v = 'd'; };
	struct CompE { std::string v = "e"; };

	// Add all components
	registry.AddComponent<CompA>(entity, CompA{});
	registry.AddComponent<CompB>(entity, CompB{});
	registry.AddComponent<CompC>(entity, CompC{});
	registry.AddComponent<CompD>(entity, CompD{});
	registry.AddComponent<CompE>(entity, CompE{});

	// Check all components exist
	EXPECT_TRUE(registry.HasComponent<CompA>(entity));
	EXPECT_TRUE(registry.HasComponent<CompB>(entity));
	EXPECT_TRUE(registry.HasComponent<CompC>(entity));
	EXPECT_TRUE(registry.HasComponent<CompD>(entity));
	EXPECT_TRUE(registry.HasComponent<CompE>(entity));

	// Remove in random order and check
	registry.RemoveComponent<CompC>(entity);
	EXPECT_FALSE(registry.HasComponent<CompC>(entity));
	registry.RemoveComponent<CompA>(entity);
	EXPECT_FALSE(registry.HasComponent<CompA>(entity));
	registry.RemoveComponent<CompE>(entity);
	EXPECT_FALSE(registry.HasComponent<CompE>(entity));
	registry.RemoveComponent<CompB>(entity);
	EXPECT_FALSE(registry.HasComponent<CompB>(entity));
	registry.RemoveComponent<CompD>(entity);
	EXPECT_FALSE(registry.HasComponent<CompD>(entity));
}

TEST(ECSTest, ViewIteratesMatchingEntities) {
	using namespace ECS;
	Registry registry;

	constexpr int N = 5;
	std::vector<Entity> entities;
	entities.reserve(N);

	struct OtherComponent { int x; OtherComponent(int v = 0) : x(v) {} };

	for (int i = 0; i < N; ++i) {
		auto e = registry.CreateEntity();
		entities.push_back(e);
		registry.AddComponent<TestComponent>(e, i);
		if ((i % 2) == 0) {
			registry.AddComponent<OtherComponent>(e, i * 10);
		}
	}

	int matchedCount = 0;
	registry.View<TestComponent, OtherComponent>([&](EntityID id, TestComponent& tc, OtherComponent& oc) {
		// Should only be called for entities that have both components (even indices)
		++matchedCount;
		EXPECT_EQ(tc.value * 10, oc.x);
		});

	// Expect 3 matches for N=5 (indices 0,2,4)
	EXPECT_EQ(matchedCount, 3);
}
