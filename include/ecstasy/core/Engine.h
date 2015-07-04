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

#include "Types.h"
#include "EntitySystem.h"
#include "Entity.h"
#include "Component.h"
#include "ComponentOperations.h"
#include "EntityOperations.h"
#include "Family.h"
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <signal11/Signal.h>

namespace ECS {
	typedef Signal11::Signal<void(Entity *, ComponentBase *)> ComponentSignal;
	typedef Signal11::Signal<void(Entity *)> EntitySignal;

	
	/** Component Pools */
	class ComponentPoolBase {
	public:
		virtual ~ComponentPoolBase() {}
		virtual void freeComponent(ComponentBase *object) = 0;
	};

	template<typename T>
	class ComponentPool : public ComponentPoolBase, public ReflectionPool<T> {
	public:
		void freeComponent(ComponentBase *object) override {
			ReflectionPool<T>::free((T*)object);
		}
	};

	/** Entity Pool */
	class EntityPool : public Pool<Entity> {
	public:
		EntityPool(int initialSize, int maxSize) : Pool<Entity>(initialSize, maxSize) {}

	protected:
		Entity *newObject() override {
			return new Entity();
		}
	};
			
	/**
	 */
	/**
	 * The heart of the Entity framework. It is responsible for keeping track of {@link Entity} and
	 * managing {@link EntitySystem} objects. The Engine should be updated every tick via the {@link #update(float)} method.
	 *
	 * With the Engine you can:
	 *
	 * <ul>
	 * <li>Add/Remove {@link Entity} objects</li>
	 * <li>Add/Remove {@link EntitySystem}s</li>
	 * <li>Obtain a list of entities for a specific {@link Family}</li>
	 * <li>Update the main loop</li>
	 * <li>Register/unregister {@link EntityListener} objects</li>
	 * </ul>
	 *
	 * Supports {@link Entity} and {@link Component} pooling. This improves performance in environments where creating/deleting
	 * entities is frequent as it greatly reduces memory allocation.
	 * <ul>
	 * <li>Create entities using {@link #createEntity()}</li>
	 * <li>Create components using {@link #createComponent(Class)}</li>
	 * <li>Components should implement the {@link Poolable} interface when in need to reset its state upon removal</li>
	 * </ul>
	 * @author David Saltares
	 * @author Stefan Bachmann
	 */
	class Engine {
	private:
		friend class ComponentOperationHandler;
		friend class Entity;

		std::vector<Entity *> entities;
		std::unordered_map<uint64_t, Entity *> entitiesById;

		std::vector<EntityOperation *> entityOperations;
		EntityOperationPool entityOperationPool;

		std::vector<EntitySystemBase *> systems;
		std::unordered_map<SystemType, EntitySystemBase *> systemsByType;

		std::unordered_map<const Family *, std::vector<Entity *>> entitiesByFamily;

		//fixme: some sort of ordering/priority for signals ?
		std::unordered_map<const Family *, EntitySignal> entityAddedSignals;
		std::unordered_map<const Family *, EntitySignal> entityRemovedSignals;

		bool updating = false;

		bool notifying = false;
		uint64_t nextEntityId = 1;

		/** Mechanism to delay component addition/removal to avoid affecting system processing */
		ComponentOperationPool componentOperationsPool;
		std::vector<ComponentOperation *> componentOperations;
		ComponentOperationHandler componentOperationHandler;

		std::vector<ComponentPoolBase *> componentPoolsByType;
		EntityPool entityPool;

	public:
		/** Will dispatch an event when a component is added. */
		ComponentSignal componentAdded;
		/** Will dispatch an event when a component is removed. */
		ComponentSignal componentRemoved;
		/** Will dispatch an event when an entity is added. */
		EntitySignal entityAdded;
		/** Will dispatch an event when an entity is removed. */
		EntitySignal entityRemoved;
		
	public:
		/**
		 * Creates a new Engine with a maximum of 100 entities and 100 components of each type. Use
		 * {@link #Engine(int, int, int, int)} to configure the entity and component pools.
		 */
		Engine () : Engine(10, 100, 10, 100) {}

		/**
		 * Creates new Engine with the specified pools size configurations.
		 * @param entityPoolInitialSize initial number of pre-allocated entities.
		 * @param entityPoolMaxSize maximum number of pooled entities.
		 * @param componentPoolInitialSize initial size for each component type pool.
		 * @param componentPoolMaxSize maximum size for each component type pool.
		 */
		Engine (int entityPoolInitialSize, int entityPoolMaxSize, int componentPoolInitialSize, int componentPoolMaxSize);

		virtual ~Engine() {
			// fixme: is this safe ?
			clear();
		}
		
		/** @return Clean {@link Entity} from the Engine pool. In order to add it to the {@link Engine}, use {@link #addEntity(Entity)}. */
		Entity *createEntity();

		/**
		 * Retrieves a new {@link Component} from the {@link Engine} pool. It will be placed back in the pool whenever it's removed
		 * from an {@link Entity} or the {@link Entity} itself it's removed.
		 */
		template<typename T>
		T *createComponent() {
			return getOrCreateComponentPool<T>()->obtain();
		}
		
		void free(ComponentBase *component);

		/**
		 * Removes all free entities and components from their pools. Although this will likely result in garbage collection, it will
		 * free up memory.
		 */
		void clearPools();
		
		void onComponentChange(Entity* entity, ComponentBase* component);

		// fixme: if an engine gets deleted before remaining entities, etc. this is currently used in the testcases, not sure if its actually useful in real world code
		void clear() {
			processComponentOperations();
			processPendingEntityOperations();
			removeAllEntities();
			clearPools();
		}

		uint64_t obtainEntityId() {
			return nextEntityId++;
		}

		/**
		 * Adds an entity to this Engine.
		 */
		void addEntity(Entity *entity);

		/**
		 * Removes an entity from this Engine.
		 */
		void removeEntity(Entity *entity);

		/**
		 * Removes all entities registered with this Engine.
		 */
		void removeAllEntities();

		Entity *getEntity(uint64_t id) const;

		const std::vector<Entity *> *getEntities() const {
			return &entities;
		}

		/**
		 * Adds the {@link EntitySystem} to this Engine.
		 */
		void addSystem(EntitySystemBase *system);

		/**
		 * Removes the {@link EntitySystem} from this Engine.
		 */
		void removeSystem(EntitySystemBase *system);

		/**
		 * Quick {@link EntitySystem} retrieval.
		 */
		template<typename T>
		T *getSystem() const {
			auto it = systemsByType.find(getSystemType<T>());
			if(it == systemsByType.end())
				return nullptr;
			return (T *) it->second;
		}

		/**
		 * @return immutable array of all entity systems managed by the {@link Engine}.
		 */
		const std::vector<EntitySystemBase *> &getSystems() const {
			return systems;
		}

		/**
		 * Returns immutable collection of entities for the specified {@link Family}. Will return the same instance every time.
		 */
		const std::vector<Entity *> *getEntitiesFor(const Family &family) {
			return registerFamily(family);
		}

		/**
		* Get the EntitySignal which emits when an entity is added to a family
		*/
		EntitySignal &getEntityAddedSignal(const Family &family);

		/**
		* Get the EntitySignal which emits when an entity is removed from a family
		*/
		EntitySignal &getEntityRemovedSignal(const Family &family);

		/**
		 * Updates all the systems in this Engine.
		 * @param deltaTime The time passed since the last frame.
		 */
		void update(float deltaTime);

	private:
		void updateFamilyMembership(Entity *entity);

		void removeEntityInternal(Entity *entity);

		void addEntityInternal(Entity *entity);

		void notifyFamilyListenersAdd(const Family &family, Entity *entity);

		void notifyFamilyListenersRemove(const Family &family, Entity *entity);

		const std::vector<Entity *> *registerFamily(const Family &family);

		void processPendingEntityOperations();

		void processComponentOperations();
		
		template<typename T>
		ComponentPool<T> *getOrCreateComponentPool() {
			auto type = getComponentType<T>();
			if (type >= componentPoolsByType.size())
				componentPoolsByType.resize(type + 1);
			auto *pool = (ComponentPool<T> *)componentPoolsByType[type];
			if (!pool) {
				pool = new ComponentPool<T>();
				componentPoolsByType[type] = pool;
			}
			return pool;
		}
	};
}