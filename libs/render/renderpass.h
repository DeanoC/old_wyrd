#pragma once
#ifndef WYRD_RENDER_RENDERPASS_H
#define WYRD_RENDER_RENDERPASS_H

#include "core/core.h"
#include "render/generictextureformat.h"
#include "render/types.h"

namespace Render {

struct RenderPass
{
	struct Target
	{
		LoadOp load;
		StoreOp store;
		GenericTextureFormat format;

		LoadOp stencilLoad; // nop except for stencil formats
		StoreOp stencilStore;
	};

	using Ptr = std::shared_ptr<RenderPass>;
	using WeakPtr = std::shared_ptr<RenderPass>;
	virtual ~RenderPass() = default;
};

}

#endif //WYRD_RENDERPASS_H
