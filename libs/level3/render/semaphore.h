#pragma once
#ifndef WYRD_RENDER_SEMAPHORE_H
#define WYRD_RENDER_SEMAPHORE_H

namespace Render {
struct Semaphore
{
	using Ptr = std::shared_ptr<Semaphore>;
	using WeakPtr = std::shared_ptr<Semaphore>;
	virtual ~Semaphore() = default;
};

}

#endif //WYRD_SEMAPHORE_H
