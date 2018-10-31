#pragma once
#ifndef WYRD_RENDER_DISPLAY_H
#define WYRD_RENDER_DISPLAY_H

#include "core/core.h"

namespace Render
{
struct Texture;

struct Display
{
	Display(uint32_t width_, uint32_t height_) : width(width_), height(height_) {}
	virtual ~Display() = default;

	auto getWidth() const -> uint32_t { return width; }
	auto getHeight() const -> uint32_t { return height; }

	virtual auto present(std::shared_ptr<Render::Texture> const& src_) -> bool = 0;

	uint32_t width, height;
};

}

#endif //WYRD_DISPLAY_H
