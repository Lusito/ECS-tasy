/*******************************************************************************
 * Copyright 2015 See AUTHORS file.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#include "../TestBase.hpp"
#include <ecstasy/systems/IteratingSystem.hpp>

#define NS_TEST_CASE(name) TEST_CASE("IteratingSystem: " name)
namespace IteratingSystemTests {
	const float deltaTime = 0.16f;

	struct ComponentA : public Component<ComponentA> {};
	struct ComponentB : public Component<ComponentB> {};
	struct ComponentC : public Component<ComponentC> {};

	class IteratingSystemMock : public IteratingSystem<IteratingSystemMock>{
	public:
		int numUpdates = 0;

		IteratingSystemMock(const Family &family) : IteratingSystem(family){}

		void processEntity (Entity* entity, float deltaTime) override {
			++numUpdates;
		}
	};

	struct SpyComponent : public Component<SpyComponent> {
		int updates = 0;
	};

	struct IndexComponent : public Component<IndexComponent> {
		int index;

		IndexComponent(int index=0) : index(index) {}
	};

	class IteratingComponentRemovalSystem : public IteratingSystem<IteratingComponentRemovalSystem> {
	public:
		IteratingComponentRemovalSystem ()
			:IteratingSystem(Family::all<SpyComponent, IndexComponent>().get()) {}

		void processEntity (Entity* entity, float deltaTime) override {
			int index = entity->get<IndexComponent>()->index;
			if (index % 2 == 0) {
				entity->remove<SpyComponent>();
				entity->remove<IndexComponent>();
			} else {
				entity->get<SpyComponent>()->updates++;
			}
		}

	};

	class IteratingRemovalSystem : public IteratingSystem<IteratingRemovalSystem> {
	public:
		Engine* engine;

		IteratingRemovalSystem ()
			:IteratingSystem(Family::all<SpyComponent, IndexComponent>().get()) {}

		void addedToEngine(Engine* engine) override {
			IteratingSystem::addedToEngine(engine);
			this->engine = engine;
		}

		void processEntity(Entity* entity, float deltaTime) override {
			int index = entity->get<IndexComponent>()->index;
			if (index % 2 == 0)
				engine->removeEntity(entity);
			else
				entity->get<SpyComponent>()->updates++;
		}
	};

	NS_TEST_CASE("shouldIterateEntitiesWithCorrectFamily") {
		TEST_MEMORY_LEAK_START
		Engine engine;

		auto &family = Family::all<ComponentA, ComponentB>().get();
		auto system = engine.emplaceSystem<IteratingSystemMock>(family);
		Entity* e = engine.createEntity();
		engine.addEntity(e);

		// When entity has ComponentA
		e->emplace<ComponentA>();
		engine.update(deltaTime);

		REQUIRE(0 == system->numUpdates);

		// When entity has ComponentA and ComponentB
		system->numUpdates = 0;
		e->emplace<ComponentB>();
		engine.update(deltaTime);

		REQUIRE(1 == system->numUpdates);

		// When entity has ComponentA, ComponentB and ComponentC
		system->numUpdates = 0;
		e->emplace<ComponentC>();
		engine.update(deltaTime);

		REQUIRE(1 == system->numUpdates);

		// When entity has ComponentB and ComponentC
		system->numUpdates = 0;
		e->remove<ComponentA>();
		engine.update(deltaTime);

		REQUIRE(0 == system->numUpdates);
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("entityRemovalWhileIterating") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		auto entities = engine.getEntitiesFor(Family::all<SpyComponent, IndexComponent>().get());

		engine.emplaceSystem<IteratingRemovalSystem>();

		int numEntities = 10;

		for (int i = 0; i < numEntities; ++i) {
			auto e = engine.createEntity();
			e->emplace<SpyComponent>();
			e->emplace<IndexComponent>(i + 1);

			engine.addEntity(e);
		}

		engine.update(deltaTime);

		REQUIRE((numEntities / 2) == entities->size());

		for (auto e : *entities) {
			REQUIRE(1 == e->get<SpyComponent>()->updates);
		}
		TEST_MEMORY_LEAK_END
	}

	NS_TEST_CASE("componentRemovalWhileIterating") {
		TEST_MEMORY_LEAK_START
		Engine engine;
		auto entities = engine.getEntitiesFor(Family::all<SpyComponent, IndexComponent>().get());

		engine.emplaceSystem<IteratingComponentRemovalSystem>();

		int numEntities = 10;

		for (int i = 0; i < numEntities; ++i) {
			auto e = engine.createEntity();
			e->emplace<SpyComponent>();
			e->emplace<IndexComponent>(i + 1);

			engine.addEntity(e);
		}

		engine.update(deltaTime);

		REQUIRE((numEntities / 2) == entities->size());

		for (auto e : *entities) {
			REQUIRE(1 == e->get<SpyComponent>()->updates);
		}
		TEST_MEMORY_LEAK_END
	}
}
