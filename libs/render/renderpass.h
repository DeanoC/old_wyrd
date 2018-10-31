#pragma once
#ifndef WYRD_RENDER_RENDERPASS_H
#define WYRD_RENDER_RENDERPASS_H

#include "core/core.h"
#include "render/generictextureformat.h"

namespace Render {

struct RenderPass
{
	enum class Load
	{
		Load,
		Clear,
		DontCare
	};

	enum class Store
	{
		Store,
		DontCare
	};

	struct Target
	{
		Load load;
		Store store;
		GenericTextureFormat format;

		Load stencilLoad; // nop except for stencil formats
		Store stencilStore;
	};

	using Ptr = std::shared_ptr<RenderPass>;
	using WeakPtr = std::shared_ptr<RenderPass>;
	virtual ~RenderPass() = default;
};

}

#endif //WYRD_RENDERPASS_H
