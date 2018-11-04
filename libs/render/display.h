#pragma once
#ifndef WYRD_RENDER_DISPLAY_H
#define WYRD_RENDER_DISPLAY_H

#include "core/core.h"

namespace Render
{
struct Texture;

struct Display
{
	using Ptr = std::shared_ptr<Display>;
	using ConstPtr = std::shared_ptr<Display const>;
	using WeakPtr = std::weak_ptr<Display>;
	using ConstWeakPtr = std::weak_ptr<Display const>;

	virtual ~Display() = default;

	auto getWidth() const -> uint32_t { return width; }
	auto getHeight() const -> uint32_t { return height; }

	virtual auto present(std::shared_ptr<Render::Texture> const& src_) -> void = 0;

protected:
	Display(uint32_t width_, uint32_t height_) : width(width_), height(height_) {}
	uint32_t width, height;

};

}

#endif //WYRD_DISPLAY_H
