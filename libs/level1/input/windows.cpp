
#include "core/core.h"
#include "input/input.h"
#include "input/keyboard.h"

namespace Input {
bool KeyboardWinProcessKeyMessages(uint32_t message, uint16_t wParam, uint32_t lParam)
{
	if(g_Keyboard == nullptr) return false;

	// Consolidate the keyboard messages and pass them to the app's keyboard callback
	if(message == WM_KEYDOWN ||
	   message == WM_SYSKEYDOWN ||
	   message == WM_KEYUP ||
	   message == WM_SYSKEYUP)
	{
		bool bKeyDown = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
		DWORD dwMask = (1 << 29);
		bool bAltDown = ((lParam & dwMask) != 0);
		Key key = Key(wParam & 0xFF);

		if(bKeyDown)
		{
			if(g_Keyboard->keyDown(key))
			{
				// still down set the held flag
				g_Keyboard->keyDataState[(uint16_t) key] |= Keyboard::KeyHeldFlag;
				uint8_t count = g_Keyboard->keyDataState[(uint16_t) key] & Keyboard::KeyDownMask;
				if(count < 255) g_Keyboard->keyDataState[(uint16_t) key] |= count + 1;
			} else
			{
				// also set held count to 0
				g_Keyboard->keyDataState[(uint16_t) key] = Keyboard::KeyDownFlag;
			}
		} else
		{
			g_Keyboard->keyDataState[(uint16_t) key] = 0;
		}

		// kill menu buttons
		g_Keyboard->keyDataState[(uint16_t) Key::KT_LMENU] = 0x80u;
		g_Keyboard->keyDataState[(uint16_t) Key::KT_RMENU] = 0x80u;
		return true;
	}
	return false;
}

} // end namespace

