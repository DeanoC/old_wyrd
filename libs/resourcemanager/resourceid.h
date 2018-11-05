#pragma once
#ifndef WYRD_RESOURCEMANAGER_RESOURCEID_H
#define WYRD_RESOURCEMANAGER_RESOURCEID_H

#include "core/core.h"

namespace ResourceManager {
/// used to identify resources, each should be unique to a project etc. IFF like
constexpr uint32_t operator "" _resource_id(char const* s, size_t count)
{
	assert(count <= 4 && count > 0);
	uint32_t res =
			(s[0] << 0) |
			((count > 1 ? s[1] : uint32_t('_')) << 8) |
			((count > 2 ? s[2] : uint32_t('_')) << 16) |
			((count > 3 ? s[3] : uint32_t('_')) << 24);
	return res;
}
}
#endif //WYRD_RESOURCEMANAGER_RESOURCEID_H
