#pragma once

#ifndef CORE_QUICK_HASH_H
#define CORE_QUICK_HASH_H

// compile time hash
#include <cstdint>
#include <string>

namespace Core
{
	// FNV-1a 32bit hashing algorithm.
	constexpr uint32_t QuickHash(char const* s, size_t count)
	{
		return ((count ? QuickHash(s, count - 1) : 2166136261ull) ^ s[count]) * 16777619ull;
	}
	inline uint32_t QuickHash(std::string s)
	{
		return QuickHash(s.c_str(), s.size());
	}
}

constexpr uint32_t operator"" _hash(char const* s, size_t count)
{
	return Core::QuickHash(s, count);
}
#endif //CORE_QUICK_HASH_H
