#pragma once
#ifndef WYRD_RENDER_STABLE_H
#define WYRD_RENDER_STABLE_H

#include "core/core.h"
#include "render/display.h"
#include "render/device.h"
#include <vector>


namespace Render {

struct DeviceConfig
{
	bool presentable  = false;
	bool renderer = true;
	bool compute = true;
	std::vector<std::string> requiredExtensions;

	// presentable parameters
	uint32_t width = 1280, height = 720;
	bool hdr = false;

	// renderer parameters

};

/// The stable holds your throughbred gpu ready to go galloping
struct Stable
{
	virtual ~Stable(){};
	virtual auto Init(std::string const& appName_, std::vector<std::string> const& backendSpecific_) -> bool = 0;
	virtual auto Finish() -> void = 0;

	virtual auto getGpuCount() const -> uint32_t = 0;
	virtual auto canGpuPresent(uint32_t index_) const -> bool = 0;
	virtual auto isGpuLowPower(uint32_t index_) const -> bool = 0;
	virtual auto createGpuDevice(uint32_t index_, DeviceConfig const& config_) -> std::shared_ptr<Render::Device> = 0;

};

}

#endif //WYRD_STABLE_H
