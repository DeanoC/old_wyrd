#pragma once
#ifndef WYRD_REPLAY_ITEMS_H
#define WYRD_REPLAY_ITEMS_H

#include "core/core.h"
#include "nlohmann/json.h"
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

constexpr ItemType TestType = "TEST"_item_type;
// test data is a C (null terminated) string

constexpr ItemType LogType = "LOG"_item_type;
// Log type data is a json object with the following fields
// <optional> level = [INFO|WARNING|ERROR] <default = ERROR>
// <optional> group = string <default = "">
// text = string
struct LogItem
{
	LogItem(nlohmann::json const& in_);

	enum class Level { Info, Warning, Error } level = Level::Error;
	std::string group;
	std::string text;
};

} // end items;

} // end replay
#endif //WYRD_REPLAY_ITEMS_H
