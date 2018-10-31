#pragma once

#ifndef WYRD_RENDER_RENDERTARGET_H
#define WYRD_RENDER_RENDERTARGET_H

namespace Render {
struct RenderTarget
{
	using Ptr = std::shared_ptr<RenderTarget>;
	using WeakPtr = std::shared_ptr<RenderTarget>;

	virtual ~RenderTarget() = default;
};

}
#endif //WYRD_RENDERTARGET_H
