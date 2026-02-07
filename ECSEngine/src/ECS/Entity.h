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
#include <string>
#include <utility>

namespace ECS
{
	class Registry;

	class Entity
	{
	public:
		/**
		 * @brief Default construct an Entity.
		 */
		Entity() = default;

		/**
		 * @brief Construct an Entity with an id and registry pointer.
		 *	This constructor is used internally by the Registry.
		 *
		 * @param id Full entity id (index + version).
		 * @param reg Pointer to the owning Registry.
		 */
		Entity(EntityID id, Registry* reg) : m_id(id), registry(reg) {}

		/**
		 * @brief Construct an Entity with an id only (no registry).
		 *	Provides a linker symbol for simple uses.
		 *
		 * @param id Full entity id.
		 */
		Entity(EntityID id) : m_id(id), registry(nullptr) {}

		Entity(const Entity& e) = default;

		/**
		 * @brief Mark the entity to be destroyed via its registry.
		 *	Equivalent to calling Registry::KillEntity on this entity.
		 */
		void Kill();

		/**
		 * @brief Get the full entity id (index + version encoded).
		 *
		 * @return u64 The full entity id.
		 */
		EntityID GetId() const;

		/**
		 * @brief Add a component of type T to this entity.
		 *
		 * Forwards arguments to the component constructor.
		 *
		 * @tparam T Component type.
		 * @tparam TArgs Constructor argument types.
		 * @param args Arguments forwarded to the component constructor.
		 */
		template<typename T, typename ...TArgs> void AddComponent(TArgs&& ...args);

		/**
		 * @brief Remove the component of type T from this entity.
		 *
		 * @tparam T Component type to remove.
		 */
		template<typename T> void RemoveComponent();

		/**
		 * @brief Check whether this entity has a component of type T.
		 *
		 * @tparam T Component type to query.
		 * @return true If the component exists.
		 * @return false Otherwise.
		 */
		template<typename T> bool HasComponent() const;

		/**
		 * @brief Get a reference to the entity's component of type T.
		 *
		 * The caller must ensure the component exists.
		 *
		 * @tparam T Component type to retrieve.
		 * @return T& Reference to the component.
		 */
		template<typename T> T& GetComponent() const;

		Entity& operator=(const Entity& e) = default;
		bool operator==(const Entity& e) const
		{
			return GetId() == e.GetId();
		}
		bool operator!=(const Entity& e) const
		{
			return GetId() != e.GetId();
		}
		bool operator>(const Entity& e) const
		{
			return GetId() > e.GetId();
		}
		bool operator<(const Entity& e) const
		{
			return GetId() < e.GetId();
		}

		// Manage entity tags
		/**
		 * @brief Assign a tag to this entity.
		 *	Wrapper around Registry::TagEntity.
		 * @param tag Tag string.
		 */
		void Tag(const std::string& tag);

		/**
		 * @brief Check whether this entity has a given tag.
		 *	Wrapper around Registry::EntityHasTag.
		 * @param tag Tag string to query.
		 * @return true If the entity has the tag.
		 * @return false Otherwise.
		 */
		bool HasTag(const std::string& tag) const;

		// Manage entity groups
		/**
		 * @brief Add this entity to a named group.
		 *	Wrapper around Registry::GroupEntity.
		 * @param groupName Group name string.
		 */
		void Group(const std::string& groupName);

		/**
		 * @brief Check whether this entity belongs to a named group.
		 *	Wrapper around Registry::EntityBelongsToGroup.
		 * @param groupName Group name string to query.
		 * @return true If the entity belongs to the group.
		 * @return false Otherwise.
		 */
		bool BelongsToGroup(const std::string& groupName) const;

		Registry* registry = nullptr;

	private:
		EntityID m_id;
	};

}