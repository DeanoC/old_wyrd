#pragma once
#ifndef WYRD_TEXTRESOURCE_H
#define WYRD_TEXTRESOURCE_H

#include "core/core.h"
#include "resourcemanager/base.h"

namespace ResourceManager
{

struct TextResource : public Resource<"TEXT"_resource_id>
{
	constexpr static uint32_t Id = "TEXT"_resource_id;

	char const* const getText() const { return (char const* const)this;}
};

}
#endif //WYRD_TEXTRESOURCE_H
