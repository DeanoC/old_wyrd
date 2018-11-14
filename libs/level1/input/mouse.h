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
	static bool WinProcessMessages(uint32_t message, uint16_t wParam, uint32_t lParam);
#endif

	Mouse() : buttonState{} {}

	bool buttonDown(MouseButton button) const { return ((buttonState[((uint16_t) button)] & ButtonDownFlag) != 0); }
	bool buttonUp(MouseButton button) const { return !buttonDown(button); }
	bool buttonHeld(MouseButton button) { return buttonState[((uint16_t) button)] & ButtonHeldFlag; };
	bool buttonDoubleClicked(MouseButton button) { return buttonState[((uint16_t) button)] & ButtonDoubleClickFlag; };

	float getMouseWheelVertical() const { return mouseWheel[0]; }
	float getMouseWheelHorizontal() const { return mouseWheel[1]; }
	float getAbsMouseX() const { return absMousePos[0]; };
	float getAbsMouseY() const { return absMousePos[1]; };

protected:
	static constexpr uint16_t ButtonDownFlag = 0x8000;
	static constexpr uint16_t ButtonHeldFlag = 0x4000;
	static constexpr uint16_t ButtonDoubleClickFlag = 0x2000;
	static constexpr uint16_t KeyDownMask = 0xFF;

	std::array<uint16_t, MaxMouseButtons> buttonState;
	std::array<float,2> absMousePos;
	std::array<float,2> mouseWheel;
};

// if no keyboard is available will be nullptr
extern Mouse* g_Mouse;

} // end Input namespace
#endif //WYRD_RENDER_MOUSE_H
