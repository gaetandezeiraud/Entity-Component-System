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

#include <cstdint>

// unsigned integers
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

// signed integers
using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

constexpr u64 u64_invalid_id{ 0xffff'ffff'ffff'ffff };
constexpr u32 u32_invalid_id{ 0xffff'ffff };
constexpr u16 u16_invalid_id{ 0xffff };
constexpr u8 u8_invalid_id{ 0xff };

using f32 = float;

// Hash (FNV-1a)
constexpr u32 HashString(const char* str)
{
	u32 hash = 2166136261u;
	while (*str)
	{
		hash ^= (u8)(*str++);
		hash *= 16777619u;
	}
	return hash;
}