#pragma once
#ifndef WYRD_RENDER_RESOURCES_H
#define WYRD_RENDER_RESOURCES_H

#include "core/core.h"

namespace ResourceManager { class ResourceMan; }
namespace Render
{
	auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_) -> void;
}


#endif //WYRD_RESOURCES_H
