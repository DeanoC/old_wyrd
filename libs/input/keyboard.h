#pragma once
#ifndef WYRD_INPUT_KEYBOARD_H
#define WYRD_INPUT_KEYBOARD_H

#include "core/core.h"
#include "input/input.h"
#include <array>


namespace Input {

class Keyboard
{
public:
#if PLATFORM == WINDOWS
	friend bool KeyboardWinProcessKeyMessages(uint32_t message, uint16_t wParam, uint32_t lParam);
#elif PLATFORM == POSIX
	friend void KeyboardX11ProcessKeyEvent( bool down, XKeyEvent* event );
#endif

	Keyboard() : keyDataState{} {}

	bool keyDown(Key key) const { return ((keyDataState[((uint16_t) key)] & KeyDownFlag) != 0); }

	bool keyUp(Key key) const { return !keyDown(key); }

	bool keyHeld(Key key) { return keyDataState[((uint16_t) key)] & KeyHeldFlag; };

	uint8_t keyHeldCount(Key key) const { return keyDataState[((uint16_t) key)] & KeyDownMask; }

	// key down only once per press
	bool keyDownOnce(Key key)
	{
		// note the short circuit order is important here don't rearrange
		return (!keyHeld(key) && keyDown(key));
	}

protected:
	static constexpr uint32_t MaxKeyCount = 256;
	static constexpr uint16_t KeyDownFlag = 0x8000;
	static constexpr uint16_t KeyHeldFlag = 0x4000;
	static constexpr uint16_t KeyDownMask = 0xFF;

	std::array<uint16_t, MaxKeyCount> keyDataState;
};

// if no keyboard is available will be nullptr
extern Keyboard* g_Keyboard;

}
#endif //WYRD_KEYBOARD_H
