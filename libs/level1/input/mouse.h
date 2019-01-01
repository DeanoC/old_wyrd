#pragma once
#ifndef WYRD_RENDER_MOUSE_H
#define WYRD_RENDER_MOUSE_H

#include "core/core.h"
#include "input/input.h"
#include <array>

namespace Input {

// this design is currently based on what imgui wants + fps camera
class Mouse
{
public:
#if PLATFORM == WINDOWS
	static bool WinProcessMessages(void* phwnd, uint32_t message, uint64_t wParam, uint64_t lParam);
#endif

	Mouse() : buttonState{}, absMousePos{}, mouseWheel{}
	{}

	bool buttonDown(MouseButton button) const { return ((buttonState[((uint16_t) button)] & ButtonDownFlag) != 0); }
	bool buttonUp(MouseButton button) const { return !buttonDown(button); }
	bool buttonHeld(MouseButton button) { return buttonState[((uint16_t) button)] & ButtonHeldFlag; };
	bool buttonDoubleClicked(MouseButton button) { return buttonState[((uint16_t) button)] & ButtonDoubleClickFlag; };

	float getMouseWheelVertical() { 
		auto ret = mouseWheel[0];
		mouseWheel[0] = 0;
		return ret;
	}
	float getMouseWheelHorizontal() {
		auto ret = mouseWheel[1];
		mouseWheel[1] = 0;
		return ret;
	}

	float getAbsMouseX() const { return absMousePos[0]; };
	float getAbsMouseY() const { return absMousePos[1]; };

	void enableRelativeMode(bool relative) 
	{ 
		relativeMode = relative; 
	}

	bool isInRelativeMode() const { return relativeMode; }

	float getRelativeMouseX() {
		auto ret = relMousePos[0];
		relMousePos[0] = 0;
		return ret; 
	};
	float getRelativeMouseY() { 
		auto ret = relMousePos[1];
		relMousePos[1] = 0;
		return ret;
	};

	// once per frame clearConsumedState should be called
	// the first client that takes the input and doesn't want
	// anybody else should call inputConsumed
	auto clearConsumedState() -> void { inputConsumedFlag = false; }
	auto inputConsumed() -> void { inputConsumedFlag = true; }
	auto isInputConsumed() const -> bool { return inputConsumedFlag; }

protected:
	static constexpr uint16_t ButtonDownFlag = 0x8000;
	static constexpr uint16_t ButtonHeldFlag = 0x4000;
	static constexpr uint16_t ButtonDoubleClickFlag = 0x2000;
	static constexpr uint16_t KeyDownMask = 0xFF;

	std::array<uint16_t, MaxMouseButtons> buttonState;
	std::array<float,2> absMousePos;
	std::array<float,2> mouseWheel;

	bool relativeMode = false;
	std::array<float, 2> relativeCenter;
	std::array<float, 2> relMousePos;

	bool inputConsumedFlag = false;
};

// if no keyboard is available will be nullptr
extern Mouse* g_Mouse;

} // end Input namespace
#endif //WYRD_RENDER_MOUSE_H
