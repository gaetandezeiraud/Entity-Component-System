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
#include "IPool.h"

namespace ECS
{
	template <typename T>
	class Pool : public IPool
	{
	public:
		/**
		 * @brief Construct a new Pool object.
		 *
		 * Initializes internal storage for packed/sparse arrays.
		 */
		Pool()
		{
			m_data.reserve(DEFAULT_CAPACITY);
			m_packed.reserve(DEFAULT_CAPACITY);

			m_sparse.resize(MAX_ENTITIES / PAGE_SIZE + 1, nullptr);
		}

		/**
		 * @brief Destroy the Pool object.
		 */
		virtual ~Pool()
		{
			for (int* page : m_sparse)
			{
				if (page)
					delete[] page;
			}
		}

		/**
		 * @brief Check whether the pool contains any components.
		 *
		 * @return true If the pool is empty.
		 * @return false Otherwise.
		 */
		bool IsEmpty() const { return m_data.empty(); }

		/**
		 * @brief Get the number of components stored in the pool.
		 *
		 * @return int Number of components in the pool.
		 */
		int GetSize() const { return (int)m_data.size(); }

		/**
		 * @brief Remove all components from the pool.
		 *	Clears internal packed and sparse arrays.
		 */
		void Clear() override
		{
			m_data.clear();
			m_packed.clear();

			for (int*& page : m_sparse)
			{
				if (page)
				{
					delete[] page;
					page = nullptr;
				}
			}
		}

		/**
		 * @brief Check whether the pool has a component for the given entity id.
		 *
		 * @param entityId Full entity id.
		 * @return true If the component exists for the entity.
		 * @return false Otherwise.
		 */
		bool Has(EntityID entityId) const
		{
			u32 index = GetEntityIndex(entityId);
			u32 page = index / PAGE_SIZE;
			u32 offset = index % PAGE_SIZE;

			if (page >= m_sparse.size() || m_sparse[page] == nullptr)
				return false;

			return m_sparse[page][offset] != -1;
		}

		/**
		 * @brief Add a component instance for an entity.
		 *
		 * @param entityId Full entity id.
		 * @param object Component instance to store.
		 */
		void Add(EntityID entityId, T object)
		{
			u32 index = GetEntityIndex(entityId);
			u32 page = index / PAGE_SIZE;
			u32 offset = index % PAGE_SIZE;

			if (page >= m_sparse.size())
			{
				assert(index < MAX_ENTITIES && "Entity index exceeds maximum allowed entities");
				m_sparse.resize(page + 1, nullptr);
			}

			if (!m_sparse[page])
			{
				m_sparse[page] = new int[PAGE_SIZE]();
				std::fill(m_sparse[page], m_sparse[page] + PAGE_SIZE, -1);
			}

			m_data.push_back(std::move(object));
			m_packed.push_back(entityId);
			m_sparse[page][offset] = (int)m_data.size() - 1;
		}

		/**
		 * @brief Set or replace the component for an entity.
		 *
		 * If a component exists it will be replaced, otherwise it will be added.
		 *
		 * @param entityId Full entity id.
		 * @param object Component instance to set.
		 */
		void Set(EntityID entityId, T object)
		{
			u32 index = GetEntityIndex(entityId);
			if (Has(entityId))
				m_data[m_sparse[index / PAGE_SIZE][index % PAGE_SIZE]] = std::move(object);
			else
				Add(entityId, std::move(object));
		}

		/**
		 * @brief Remove the component for an entity.
		 *
		 * Performs swap-and-pop to keep the packed array contiguous.
		 *
		 * @param entityId Full entity id.
		 */
		void Remove(EntityID entityId)
		{
			if (!Has(entityId))
				return;

			u32 index = GetEntityIndex(entityId);
			u32 page = index / PAGE_SIZE;
			u32 offset = index % PAGE_SIZE;

			int indexToRemove = m_sparse[page][offset];
			int indexLast = (int)m_data.size() - 1;

			if (indexToRemove != indexLast)
			{
				u64 lastEntityId = m_packed[indexLast];
				u32 lastEntityIndex = GetEntityIndex(lastEntityId);

				// We move the latest element into the new free space
				m_data[indexToRemove] = std::move(m_data[indexLast]);
				m_packed[indexToRemove] = lastEntityId;

				// Update sparce with the moved entity
				m_sparse[lastEntityIndex / PAGE_SIZE][lastEntityIndex % PAGE_SIZE] = indexToRemove;
			}

			m_data.pop_back();
			m_packed.pop_back();
			m_sparse[page][offset] = -1;
		}

		/**
		 * @brief Remove an entity's component from the pool (IPool override).
		 *
		 * @param entityId Full entity id.
		 */
		void RemoveEntityFromPool(EntityID entityId) override
		{
			Remove(entityId);
		}

		/**
		 * @brief Retrieve a reference to the component for an entity.
		 *
		 * The caller must ensure the component exists in the pool.
		 *
		 * @param entityId Full entity id.
		 * @return T& Reference to the component.
		 */
		T& Get(EntityID entityId)
		{
			u32 index = GetEntityIndex(entityId);
			u32 page = index / PAGE_SIZE;
			u32 offset = index % PAGE_SIZE;

			return m_data[m_sparse[page][offset]];
		}

		/**
		 * @brief Access component data by packed index.
		 *
		 * @param index Packed array index.
		 * @return T& Reference to the component at the packed index.
		 */
		T& operator[](unsigned int index)
		{
			return m_data[index];
		}

		/**
		 * @brief Get direct access to the internal packed component array.
		 *
		 * @return std::vector<T>& Reference to the packed data vector.
		 */
		std::vector<T>& GetData() { return m_data; }

		/**
		 * @brief Get the list of entity ids that correspond to the packed data.
		 *
		 * @return const std::vector<u64>& Reference to the packed entity id list.
		 */
		const std::vector<EntityID>& GetEntities() const { return m_packed; }

	private:
		std::vector<T> m_data; // What (Packed index: Packed index -> Component)
		std::vector<EntityID> m_packed; // Who (Packed index: Packed index -> EntityID)
		std::vector<int*> m_sparse; // Where (Paging sparse index: EntityID -> Packed index, -1 if not present)
	};
}