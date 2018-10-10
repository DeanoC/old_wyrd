#pragma once
#ifndef WYRD_RENDER_DEVICE_H
#define WYRD_RENDER_DEVICE_H

#include "core/core.h"
#include "render/display.h"

namespace Render
{

struct Device
{
	virtual ~Device(){};
	virtual auto getDisplay() const -> std::shared_ptr<Display> = 0;
};

}

#endif //WYRD_GPU_DEVICE_H
