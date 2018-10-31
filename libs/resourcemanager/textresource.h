#pragma once
#ifndef WYRD_TEXTRESOURCE_H
#define WYRD_TEXTRESOURCE_H

#include "core/core.h"
#include "resourcemanager/base.h"

namespace ResourceManager
{
class ResourceMan;

struct TextResource : public Resource<"TEXT"_resource_id>
{
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	auto static RegisterResourceHandler( ResourceManager::ResourceMan& rm_ ) -> void;

	char const* const getText() const { return (char const* const) this + sizeof(ResourceBase); }
};

}
#endif //WYRD_TEXTRESOURCE_H
