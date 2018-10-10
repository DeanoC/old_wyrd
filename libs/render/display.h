#pragma once
#ifndef WYRD_RENDER_DISPLAY_H
#define WYRD_RENDER_DISPLAY_H

#include "core/core.h"

namespace Render
{
struct Display
{
	auto getWidth() const -> uint32_t { return width; }
	auto getHeight() const -> uint32_t { return height; }

	uint32_t width, height;
};

}

#endif //WYRD_DISPLAY_H
