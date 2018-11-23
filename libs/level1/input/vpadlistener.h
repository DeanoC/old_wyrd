#pragma once
#ifndef WYRD_INPUT_VPADLISTENER_H
#define WYRD_INPUT_VPADLISTENER_H

#include <cstdint>

namespace Input
{

// L = Left, classically movement axises (WASD)
// R = Right classically rotation axises (Mouse look)
// LZ = Left Trigger / MouseWheel, RZ = Right Trigger
enum class VPadAxisId : uint8_t
{
	LX = 0,
	LY,
	LZ,
	RX,
	RY,
	RZ
};

enum class VPadButtonId : uint8_t
{
	First = 0, 	// Primary  / Left Mouse
	Second,		// Secondary / Right Mouse
	Third,		// 3rd / Mouse Wheel click
	Fourth
};


struct VPadListener
{
	virtual ~VPadListener(){};

	virtual auto axisMovement( VPadAxisId id_, float delta_ ) -> void {}
	virtual auto button( VPadButtonId id_, float delta_ ) -> void {}

};

}

#endif //WYRD_LISTENER_H
