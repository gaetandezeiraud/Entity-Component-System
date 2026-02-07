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
#include "Component.h"

namespace ECS
{
	class System
	{
	public:
		/**
		 * @brief Construct a new System object.
		 */
		System() = default;

		/**
		 * @brief Destroy the System object.
		 */
		~System() = default;

		/**
		 * @brief Add an entity to the system's internal list.
		 *	This is used by the Registry when an entity matches the system's signature.
		 * @param entity The entity to add.
		 */
		void AddEntityToSystem(Entity entity);

		/**
		 * @brief Remove an entity from the system's internal list.
		 *	Called when an entity is killed or no longer matches the system signature.
		 * @param entity The entity to remove.
		 */
		void RemoveEntityFromSystem(Entity entity);

		/**
		 * @brief Get the list of entities currently in the system.
		 *
		 * @return const std::vector<Entity>& Reference to the entities vector.
		 */
		const std::vector<Entity>& GetSystemEntities() const;

		/**
		 * @brief Get the component signature required by the system.
		 *
		 * @return const Signature& Reference to the component signature bitset.
		 */
		const Signature& GetComponentSignature() const;

		virtual void Add(Entity entity) {};
		virtual void Remove(Entity entity) {};

		template<typename T> void RequireComponent();

	private:
		Signature m_componentSignature;
		std::vector<Entity> m_entities;
	};

	template<typename T>
	void System::RequireComponent()
	{
		const auto componentId = Component<T>::GetId();
		m_componentSignature.set(componentId);
	}
}