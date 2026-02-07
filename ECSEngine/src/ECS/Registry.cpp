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

#include "Registry.h"
#include "Component.h"

namespace ECS
{
	// Entity implementation moved to Entity.h

	void Entity::Kill()
	{
		registry->KillEntity(*this);
	}

	EntityID Entity::GetId() const
	{
		return m_id;
	}

	void Registry::Update()
	{
		// Add the entities that are waiting to be created to the active Systems
		for (auto e : m_entitiesToBeAdded)
			AddEntityToSystems(e);
		m_entitiesToBeAdded.clear();

		// Process the entities that are waiting to be killed
		for (auto e : m_entitiesToBeKilled)
		{
			if (!IsValid(e))
				continue;

			u32 index = GetEntityIndex(e.GetId());


			RemoveEntityFromSystems(e);
			m_entityComponentSignatures[index].reset();

			// Remove the entity from the component pools
			for (auto& pool : m_componentPools)
			{
				if (pool)
					pool->RemoveEntityFromPool(e.GetId());
			}

			RemoveEntityTag(e);
			RemoveEntityGroup(e);

			m_entityVersions[index]++;

			// Make the entity id available to be reused
			m_freeIndices.emplace_back(index);
		}
		m_entitiesToBeKilled.clear();
	}

	void Registry::AddEntityToSystems(Entity e)
	{
		const auto entityId = e.GetId();

		u32 index = GetEntityIndex(entityId);
		const auto& entityComponentSignature = m_entityComponentSignatures[index];

		for (auto& system : m_systems)
		{
			const auto& systemComponentSignature = system.second->GetComponentSignature();
			bool isInterested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;

			if (isInterested)
				system.second->AddEntityToSystem(e);
		}
	}

	void Registry::RemoveEntityFromSystems(Entity e)
	{
		for (auto& system : m_systems)
			system.second->RemoveEntityFromSystem(e);
	}

	Entity Registry::CreateEntity()
	{
		u32 index;

		if (m_freeIndices.empty())
		{
			// No free ids, so we add one more
			index = m_numEntities++;

			if (index >= m_entityVersions.size())
			{
				m_entityVersions.resize(index + 1, 0);
				if (index >= m_entityComponentSignatures.size())
					m_entityComponentSignatures.resize(index + 1);
			}
		}
		else
		{
			index = m_freeIndices.front();
			m_freeIndices.pop_front();
		}

		// Build the entity id by combining the index and the version
		u32 version = m_entityVersions[index];
		EntityID id = CreateEntityId(index, version);

		Entity entity(id, this);
		m_entitiesToBeAdded.insert(entity);

		return entity;
	}

	void Registry::KillEntity(Entity e)
	{
		if (!IsValid(e))
			return;

		m_entitiesToBeKilled.insert(e);
	}

	bool Registry::IsValid(Entity e) const
	{
		u32 index = GetEntityIndex(e.GetId());
		if (index >= m_entityVersions.size())
			return false;

		// Valid only if the version matches the one stored in the registry (if not, it means the entity was killed and its id was reused for a new entity)
		return m_entityVersions[index] == GetEntityVersion(e.GetId());
	}

	// Tag management
	void Registry::TagEntity(Entity e, const char* tag)
	{
		u32 index = GetEntityIndex(e.GetId());
		u32 hash = HashString(tag);

		// Sparse Vector (Entity -> Hash)
		if (index >= m_entityToTag.size())
			m_entityToTag.resize(index + 1, 0);

		m_entityToTag[index] = hash;

		// Map (Hash -> Entity)
		m_tagToEntity[hash] = e.GetId();
	}

	bool Registry::EntityHasTag(Entity e, const char* tag) const
	{
		u32 index = GetEntityIndex(e.GetId());

		if (index >= m_entityToTag.size())
			return false;

		return m_entityToTag[index] == HashString(tag);
	}

	Entity Registry::GetEntityByTag(const char* tag)
	{
		u32 hash = HashString(tag);
		auto it = m_tagToEntity.find(hash);
		if (it != m_tagToEntity.end())
			return Entity(it->second, this);

		return Entity(0, nullptr);
	}

	void Registry::RemoveEntityTag(Entity e)
	{
		u32 index = GetEntityIndex(e.GetId());

		if (index >= m_entityToTag.size())
			return;

		u32 hash = m_entityToTag[index];
		if (hash != 0)
		{
			m_tagToEntity.erase(hash);
			m_entityToTag[index] = 0;
		}
	}

	// Group management
	void Registry::GroupEntity(Entity e, const char* groupName)
	{
		u32 groupHash = HashString(groupName);
		GroupData& group = m_groups[groupHash];
		EntityID id = e.GetId();
		u32 index = GetEntityIndex(e.GetId());

		// Avoid duplication in a group
		if (group.entityToIndex.find(id) != group.entityToIndex.end())
			return;

		group.entities.push_back(e);
		group.entityToIndex[id] = (int)group.entities.size() - 1;

		// Reverse Lookup Optimization
		if (index >= m_entityGroups.size())
			m_entityGroups.resize(index + 1);

		m_entityGroups[index].push_back(groupHash);
	}

	bool Registry::EntityBelongsToGroup(Entity e, const char* groupName) const
	{
		u32 hash = HashString(groupName);
		auto it = m_groups.find(hash);
		if (it == m_groups.end())
			return false;

		return it->second.entityToIndex.find(e.GetId()) != it->second.entityToIndex.end();
	}

	const std::vector<Entity>& Registry::GetEntitiesByGroup(const char* groupName)
	{
		return m_groups[HashString(groupName)].entities;
	}

	void Registry::RemoveEntityGroup(Entity e)
	{
		EntityID id = e.GetId();
		u32 index = GetEntityIndex(id);

		if (index >= m_entityGroups.size()) return;

		// Only on groups of the entity
		std::vector<u32>& groupsOfEntity = m_entityGroups[index];

		for (u32 groupHash : groupsOfEntity)
		{
			auto itGroup = m_groups.find(groupHash);
			if (itGroup != m_groups.end())
			{
				GroupData& group = itGroup->second;
				auto itIndex = group.entityToIndex.find(id);

				if (itIndex != group.entityToIndex.end())
				{
					// Swap & Pop Logic
					int indexToRemove = itIndex->second;
					int indexLast = (int)group.entities.size() - 1;

					if (indexToRemove != indexLast)
					{
						Entity lastEntity = group.entities[indexLast];
						group.entities[indexToRemove] = lastEntity;
						group.entityToIndex[lastEntity.GetId()] = indexToRemove;
					}

					group.entities.pop_back();
					group.entityToIndex.erase(itIndex);
				}
			}
		}

		groupsOfEntity.clear();
	}

	// Manage entity tags
	void Entity::Tag(const std::string& tag)
	{
		registry->TagEntity(*this, tag.c_str());
	}

	bool Entity::HasTag(const std::string& tag) const
	{
		return registry->EntityHasTag(*this, tag.c_str());
	}

	// Manage entity groups
	void Entity::Group(const std::string& groupName)
	{
		registry->GroupEntity(*this, groupName.c_str());
	}

	bool Entity::BelongsToGroup(const std::string& groupName) const
	{
		return registry->EntityBelongsToGroup(*this, groupName.c_str());
	}
}