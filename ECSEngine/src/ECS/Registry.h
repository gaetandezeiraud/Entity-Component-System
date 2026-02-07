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

#pragma once

#include "Common.h"
#include "Entity.h"
#include "Pool.h"
#include "System.h"
#include "Component.h"

namespace ECS
{

	class Registry
	{
	public:
		/**
		 * @brief Construct a new Registry object.
		 *
		 * The Registry manages entities, components, systems, tags and groups.
		 */
		Registry() = default;

		/**
		 * @brief Apply pending entity creation and destruction and update internal state.
		 *
		 * This will process entities queued via CreateEntity() / KillEntity() and
		 * update system membership, free index pools and internal versioning.
		 */
		void Update();

		// Entity management
		/**
		 * @brief Create a new entity.
		 *
		 * The created entity may not become active until Update() is called if the
		 * registry defers creation. The returned Entity contains a full id that
		 * encodes index and version information.
		 *
		 * @return Entity The newly created entity.
		 */
		Entity CreateEntity();

		/**
		 * @brief Mark an entity to be destroyed.
		 *
		 * The entity will be removed during the next Update() call. Components and
		 * system memberships for the entity will be cleaned up at that time.
		 *
		 * @param e The entity to destroy.
		 */
		void KillEntity(Entity e);

		/**
		 * @brief Iterate over all entities that have a specific set of components and apply a function to them.
		 *
		 * @note CRITICAL PERFORMANCE NOTE: The order of component types MATTERS.
		 * The View iterates over the entities of the FIRST component type specified (the "Leader").
		 * You should always list the rarest component first to minimize iterations.
		 *
		 * Example:
		 * - View<Transform, Player>: Iterates over ALL Transforms (e.g., 10 000 entities) -> Checks for Player.
		 * -> Slow (10 000 checks).
		 * - View<Player, Transform>: Iterates over ONLY Players (e.g., 1 entity) -> Checks for Transform.
		 * -> Fast (1 check).
		 *
		 * In this example, <Player, Transform> is 10 000x faster than <Transform, Player>.
		 *
		 * @tparam Component The list of component types. Put the rarest one first!
		 * @tparam Func The function (lambda) to execute on each match.
		 * @param func The lambda function taking (EntityID, Component&...).
		 */
		template<typename... Component, typename Func> void View(Func&& func);

		// Component management
		/**
		 * @brief Add a component of type T to an entity.
		 *
		 * Constructs the component in the component pool using the provided
		 * constructor arguments.
		 *
		 * @tparam T Component type to add.
		 * @tparam TArgs Constructor argument types for T.
		 * @param e The entity to which the component will be added.
		 * @param args Arguments forwarded to T's constructor.
		 */
		template<typename T, typename ...TArgs> void AddComponent(Entity e, TArgs&& ...args);

		/**
		 * @brief Remove a component of type T from an entity.
		 *
		 * @tparam T Component type to remove.
		 * @param e The entity from which the component will be removed.
		 */
		template<typename T> void RemoveComponent(Entity e);

		/**
		 * @brief Check whether an entity has a component of type T.
		 *
		 * @tparam T Component type to query.
		 * @param e The entity to check.
		 * @return true If the entity has the component.
		 * @return false Otherwise.
		 */
		template<typename T> bool HasComponent(Entity e) const;

		/**
		 * @brief Get a reference to an entity's component of type T.
		 *
		 * The caller must ensure the component exists (e.g. by calling
		 * HasComponent<T>()).
		 *
		 * @tparam T Component type to retrieve.
		 * @param e The entity that owns the component.
		 * @return T& Reference to the component instance.
		 */
		template<typename T> T& GetComponent(Entity e) const;

		/**
		 * @brief Check whether an Entity is currently valid (alive and matches version).
		 *
		 * @param e The entity to validate.
		 * @return true If the entity is valid.
		 * @return false If the entity is invalid (killed or stale id).
		 */
		bool IsValid(Entity e) const;

		// System management
		/**
		 * @brief Add a system of type T to the registry.
		 *
		 * The system instance is constructed with the provided arguments and stored
		 * internally. Systems are identified by their C++ type.
		 *
		 * @tparam T System type to add.
		 * @tparam TArgs Constructor argument types for T.
		 * @param args Arguments forwarded to T's constructor.
		 */
		template<typename T, typename ...TArgs> void AddSystem(TArgs&& ...args);

		/**
		 * @brief Remove a system of type T from the registry.
		 *
		 * @tparam T System type to remove.
		 */
		template<typename T> void RemoveSystem();

		/**
		 * @brief Check whether a system of type T exists.
		 *
		 * @tparam T System type to query.
		 * @return true If the system exists.
		 * @return false Otherwise.
		 */
		template<typename T> bool HasSystem() const;

		/**
		 * @brief Get a reference to a system of type T.
		 *
		 * The system must exist prior to calling this method.
		 *
		 * @tparam T System type to retrieve.
		 * @return T& Reference to the system instance.
		 */
		template<typename T> T& GetSystem() const;

		// Tag management
		/**
		 * @brief Assign a text tag to an entity.
		 *
		 * Tags provide a single-name mapping to an entity.
		 *
		 * @param e The entity to tag.
		 * @param tag Null-terminated C-string tag name.
		 */
		void TagEntity(Entity e, const char* tag);

		/**
		 * @brief Check whether an entity has a specific tag.
		 *
		 * @param e The entity to query.
		 * @param tag Tag name to check.
		 * @return true If the entity has the tag.
		 * @return false Otherwise.
		 */
		bool EntityHasTag(Entity e, const char* tag) const;

		/**
		 * @brief Get the entity associated with a tag.
		 *
		 * @param tag Tag name to lookup.
		 * @return Entity The entity associated with the tag. If no entity is
		 * associated the returned Entity may be invalid.
		 */
		Entity GetEntityByTag(const char* tag);

		/**
		 * @brief Remove any tag assigned to an entity.
		 *
		 * @param e The entity whose tag will be removed.
		 */
		void RemoveEntityTag(Entity e);

		// Group management
		/**
		 * @brief Add an entity to a named group.
		 *
		 * Groups allow multiple entities to be associated with a single name.
		 *
		 * @param e The entity to add to the group.
		 * @param group Null-terminated C-string group name.
		 */
		void GroupEntity(Entity e, const char* group);

		/**
		 * @brief Query whether an entity belongs to a named group.
		 *
		 * @param e The entity to query.
		 * @param group Group name to check.
		 * @return true If the entity is a member of the group.
		 * @return false Otherwise.
		 */
		bool EntityBelongsToGroup(Entity e, const char* group) const;

		/**
		 * @brief Retrieve all entities that belong to a named group.
		 *
		 * @param group Group name to lookup.
		 * @return const std::vector<Entity>& Reference to the group's entity list.
		 */
		const std::vector<Entity>& GetEntitiesByGroup(const char* group);

		/**
		 * @brief Remove an entity from all groups it belongs to.
		 *
		 * @param e The entity to remove from groups.
		 */
		void RemoveEntityGroup(Entity e);

	private:
		void AddEntityToSystems(Entity e);
		void RemoveEntityFromSystems(Entity e);

		template<typename T> Pool<T>* GetPool() const;

	private:
		int m_numEntities = 0;
		std::set<Entity> m_entitiesToBeAdded; // Entities awaiting creation in the next Registry Update()
		std::set<Entity> m_entitiesToBeKilled; // Entities awaiting destruction in the next Registry Update()

		// Vector of component pools.
		// Each pool contains all the data for a specific component type
		// [vector index = componentId], [pool index = entity Id]
		std::vector<std::unique_ptr<IPool>> m_componentPools;

		// Vector of component signatures.
		// The signature lets us know which components are turned "on" for an entity
		// [vector index = entity Id]
		std::vector<Signature> m_entityComponentSignatures;

		// Map of active systems [index = system typeid]
		std::unordered_map<std::type_index, std::shared_ptr<System>> m_systems;

		// List of free ids that were previously removed
		std::deque<u32> m_freeIndices;

		// Actual version of each entity (used to check if an entity is still valid after being killed and its ID reused)
		std::vector<u32> m_entityVersions;

		// Entity tags (one tag name per entity)
		// Index = EntityID, Value = TagHash
		std::vector<u32> m_entityToTag;
		// Key = TagHash, Value = EntityID
		std::unordered_map<u32, EntityID> m_tagToEntity;

		// Entity groups (a set of entities per group name)
		struct GroupData
		{
			std::vector<Entity> entities;
			std::unordered_map<EntityID, int> entityToIndex;
		};
		std::unordered_map<u32, GroupData> m_groups;
		// ID -> Groups
		std::vector<std::vector<u32>> m_entityGroups;
	};

	template<typename... Components, typename Func>
	void Registry::View(Func&& func)
	{
		if ((!GetPool<Components>() || ...))
			return;

		auto pools = std::make_tuple(GetPool<Components>()...);

		using FirstType = std::tuple_element_t<0, std::tuple<Components...>>;
		auto* leaderPool = std::get<0>(pools);

		const std::vector<EntityID>& entities = leaderPool->GetEntities();

		for (EntityID entityId : entities)
		{
			// Fold expression to check if the entity has all required components
			bool hasAll = std::apply([entityId](auto*... p) {
				return (... && p->Has(entityId));
				}, pools);

			if (hasAll)
				func(entityId, std::get<Pool<Components>*>(pools)->Get(entityId)...);
		}

	}

	template<typename T, typename ...TArgs>
	void Registry::AddComponent(Entity e, TArgs&& ...args)
	{
		const auto componentId = Component<T>::GetId();
		const auto entityId = e.GetId();

		if (componentId >= m_componentPools.size())
			m_componentPools.resize(componentId + 1);

		// If we still don't have a Pool for that component type
		if (!m_componentPools[componentId])
			m_componentPools[componentId] = std::make_unique<Pool<T>>();

		// Get the pool of component values for that component type
		auto* pool = static_cast<Pool<T>*>(m_componentPools[componentId].get());
		pool->Add(entityId, T(std::forward<TArgs>(args)...));

		m_entityComponentSignatures[entityId].set(componentId);

	}

	template<typename T>
	void Registry::RemoveComponent(Entity e)
	{
		const auto componentId = Component<T>::GetId();
		const auto entityId = e.GetId();

		m_componentPools[componentId]->RemoveEntityFromPool(entityId);

		m_entityComponentSignatures[entityId].set(componentId, false);
	}

	template<typename T>
	bool Registry::HasComponent(Entity e) const
	{
		const auto componentId = Component<T>::GetId();
		const auto entityId = e.GetId();

		return m_entityComponentSignatures[entityId].test(componentId);
	}

	template<typename T>
	T& Registry::GetComponent(Entity e) const
	{
		const auto componentId = Component<T>::GetId();
		auto* pool = static_cast<Pool<T>*>(m_componentPools[componentId].get());

		return pool->Get(e.GetId());
	}

	// System management
	template<typename T, typename ...TArgs>
	void Registry::AddSystem(TArgs&& ...args)
	{
		auto newSystem = std::make_shared<T>(std::forward<TArgs>(args)...);
		m_systems.insert(std::make_pair(std::type_index(typeid(T)), newSystem));
	}

	template<typename T>
	void Registry::RemoveSystem()
	{
		auto system = m_systems.find(std::type_index(typeid(T)));
		m_systems.erase(system);
	}

	template<typename T>
	bool Registry::HasSystem() const
	{
		return m_systems.find(std::type_index(typeid(T))) != m_systems.end();
	}

	template<typename T>
	T& Registry::GetSystem() const
	{
		auto system = m_systems.find(std::type_index(typeid(T)));
		return *(std::static_pointer_cast<T>(system->second));
	}

	template<typename T>
	Pool<T>* Registry::GetPool() const
	{
		const auto componentId = Component<T>::GetId();
		if (componentId >= m_componentPools.size() || !m_componentPools[componentId])
			return nullptr;

		return static_cast<Pool<T>*>(m_componentPools[componentId].get());
	}
}

#include "Entity.inl"