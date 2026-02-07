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

#include "PrivimiteTypes.h"

#include <bitset>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include <deque>
#include <cassert>

namespace ECS
{
	constexpr unsigned int MAX_COMPONENTS = 32;
	constexpr unsigned int MAX_ENTITIES = 1000000;
	constexpr unsigned int DEFAULT_CAPACITY = 1000;
	constexpr size_t PAGE_SIZE = 4096;

	using EntityID = u64;

	constexpr u64 ENTITY_INDEX_MASK = 0xFFFFFFFF;
	constexpr u64 ENTITY_VERSION_MASK = 0xFFFFFFFF00000000;
	constexpr int ENTITY_VERSION_SHIFT = 32;

	// We use a bitset (1s and 0s) to keep track of which components an entity has,
	// and also helps keep track of which entities a system is interested in.
	typedef std::bitset<MAX_COMPONENTS> Signature;

	inline u32 GetEntityIndex(EntityID id) { return id & ENTITY_INDEX_MASK; }
	inline u32 GetEntityVersion(EntityID id) { return (id & ENTITY_VERSION_MASK) >> ENTITY_VERSION_SHIFT; }
	inline EntityID CreateEntityId(u32 index, u32 version) { return ((u64)version << ENTITY_VERSION_SHIFT) | index; }
}