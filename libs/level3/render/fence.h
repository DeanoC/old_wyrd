#pragma once

#ifndef WYRD_RENDER_FENCE_H
#define WYRD_RENDER_FENCE_H

namespace Render {
struct Fence
{
	using Ptr = std::shared_ptr<Fence>;
	using WeakPtr = std::shared_ptr<Fence>;

	virtual ~Fence() = default;
};

}

#endif //WYRD_FENCE_H
