#pragma once
/*******************************************************************************
* Copyright 2014 See AUTHORS file.
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
#include <vector>
#include "IntervalSystem.h"
#include "../core/Family.h"
#include "../core/Engine.h"

namespace ECS {
	class Entity;

	/**
	 * A simple {@link EntitySystem} that processes a {@link Family} of entities not once per frame, but after a given interval.
	 * Entity processing logic should be placed in {@link IntervalIteratingSystem#processEntity(Entity)}.
	 * @author David Saltares
	 */
	template<typename T>
	class IntervalIteratingSystem: public IntervalSystem<T> {
	private:
		Family &family;
		const std::vector<Entity *> *entities;

	public:
		/**
		 * @param family represents the collection of family the system should process
		 * @param interval time in seconds between calls to {@link IntervalIteratingSystem#updateInterval()}.
		 * @param priority
		 */
		IntervalIteratingSystem(Family &family, float interval, int priority = 0) : IntervalSystem<T>(interval, priority), family(family) {}

		void addedToEngine(Engine *engine) override {
			entities = engine->getEntitiesFor(family);
		}

	protected:
		void updateInterval() override {
			for (auto entity: *entities) {
				processEntity(entity);
			}
		}

	public:
		/**
		 * @return set of entities processed by the system
		 */
		const std::vector<Entity *> *getEntities() {
			return entities;
		}

		/**
		 * @return the Family used when the system was created
		 */
		Family &getFamily() {
			return family;
		}

	protected:
		/**
		 * The user should place the entity processing logic here.
		 * @param entity
		 */
		virtual void processEntity(Entity *entity) = 0;
	};
}