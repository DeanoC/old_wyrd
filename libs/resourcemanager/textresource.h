#pragma once
#ifndef WYRD_RESOURCEMANAGER_TEXTRESOURCE_H
#define WYRD_RESOURCEMANAGER_TEXTRESOURCE_H

#include "core/core.h"
#include "resourcemanager/resourceid.h"
#include "resourcemanager/resource.h"
#include <string_view>

namespace ResourceManager
{
class ResourceMan;

struct TextResource;
struct ResourceNameView;

static constexpr ResourceId TextResourceId = "TEXT"_resource_id;
using TextResourceHandle = ResourceHandle<TextResourceId>;
using TextResourcePtr = std::shared_ptr<TextResource>;

struct TextResource : public Resource<TextResourceId>
{
	auto static RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   std::string_view text_) -> TextResourceHandle;

	char const* const getText() const { return (char const* const) this + sizeof(ResourceBase); }

	size_t const getTextSize() const { return getSize() - sizeof(ResourceBase); }
};

}
#endif //WYRD_RESOURCEMANAGER_TEXTRESOURCE_H
