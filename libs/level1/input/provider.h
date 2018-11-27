#pragma once
#ifndef WYRD_INPUT_PROVIDER_H
#define WYRD_INPUT_PROVIDER_H

#include "core/core.h"
namespace Input {

struct VPadListener;

enum class VirtualPadType : uint8_t
{
	KeyboardMouse,
	Joypad,
};

struct Provider
{
#if PLATFORM == WINDOWS
	static auto WinCreateProvider() -> std::unique_ptr<Provider>;
#endif
#if PLATFORM == APPLE_MAC
	static auto MacCreateProvider() -> std::unique_ptr<Provider>;
#endif
	virtual auto getNumVirtualPads() -> uint32_t = 0;
	virtual auto setVirtualPadListener(uint32_t padIndex_, std::shared_ptr<VPadListener> const& listener_) -> void = 0;
	virtual auto getVirtualPadType(uint32_t padIndex_) -> VirtualPadType = 0;

	virtual auto update(double deltaT_) -> void = 0;
};

}

#endif //WYRD_PROVIDER_H
