#pragma once
#ifndef WYRD_REPLAY_ITEMS_H
#define WYRD_REPLAY_ITEMS_H

#include "core/core.h"
#include "nlohmann/json.h"
#include "math/vector_math.h"

namespace Replay
{
enum class ItemType : uint32_t;

namespace Items {
/// used to identify type of Item
constexpr ItemType operator "" _item_type(char const* s, size_t count)
{
	assert(count <= 4 && count > 0);
	uint32_t res =
			(s[0] << 0) |
			((count > 1 ? s[1] : uint32_t('_')) << 8) |
			((count > 2 ? s[2] : uint32_t('_')) << 16) |
			((count > 3 ? s[3] : uint32_t('_')) << 24);
	return (ItemType) res;
}

// test data is a C (null terminated) string
constexpr ItemType TestType = "TEST"_item_type;

// payloads all json objects with indicated fields

// text = string
// level = optional [info|warning|error]
// position = optional float * 3 position in world space
constexpr ItemType LogType = "LOG"_item_type;

// SMSH has two forms
// first is the actual data, second is just the name
// if the second is used, it either uses a previous defined
// mesh of the same name or a standard mesh
// name = string
// positioncount = number
// position = array of numbers (3 per positionCount)
// trianglecount = number
// indices = array of numbers (3 per triangleCount)
constexpr ItemType SimpleMeshType = "SMSH"_item_type;

// name = string
// meshname = optional string
// enable = optional bool (default = true)
// position = optional vec3
// scale = optional vec3
// rotation = optional vec3 (xyz euler)
constexpr ItemType MeshObjectType = "MSHO"_item_type;


} // end items;

} // end replay
#endif //WYRD_REPLAY_ITEMS_H
