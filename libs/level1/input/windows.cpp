#include "core/core.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/provider.h"
#include "keyboardmouselistenerimpl.h"

namespace Input {
bool Keyboard::WinProcessMessages(uint32_t message, uint16_t wParam, uint32_t lParam)
{
	if(g_Keyboard == nullptr) return false;

	/* TODO
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			return 0;
	 */
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
			g_Keyboard->keyDownBitMap[(uint16_t)key] = true;
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
			g_Keyboard->keyDownBitMap[(uint16_t)key] = false;
			g_Keyboard->keyDataState[(uint16_t) key] = 0;
		}
		return true;
	}
	return false;
}

bool Mouse::WinProcessMessages(uint32_t message, uint16_t wParam, uint32_t lParam)
{
	if(g_Mouse == nullptr) return false;

	switch (message)
	{
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		{
			uint32_t button = ~0;
			if(message == WM_LBUTTONDOWN) button = 0;
			if(message == WM_RBUTTONDOWN) button = 1;
			if(message == WM_MBUTTONDOWN) button = 2;

			if(button != ~0)
			{
				if(g_Mouse->buttonDown(MouseButton(button)))
				{
					g_Mouse->buttonState[button] |= Mouse::ButtonHeldFlag;
				}
				else
				{
					g_Mouse->buttonState[button] |= Mouse::ButtonDownFlag;
				}
			}
			uint32_t dbutton = ~0;
			if(message == WM_LBUTTONDBLCLK) dbutton = 0;
			if(message == WM_RBUTTONDBLCLK) dbutton = 1;
			if(message == WM_MBUTTONDBLCLK) dbutton = 2;
			if(dbutton != ~0)
			{
				g_Mouse->buttonState[button] |= Mouse::ButtonDoubleClickFlag;
			}
			return true;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			uint32_t button = ~0;
			if(message == WM_LBUTTONUP) button = 0;
			if(message == WM_RBUTTONUP) button = 1;
			if(message == WM_MBUTTONUP) button = 2;
			if(button != ~0)
			{
				g_Mouse->buttonState[button] = 0;
			}
			return true;
		}
		case WM_MOUSEWHEEL:
			g_Mouse->mouseWheel[0] += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			return true;
		case WM_MOUSEHWHEEL:
			g_Mouse->mouseWheel[1] += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			return true;
		case WM_MOUSEMOVE:
		{
			int xPosAbsolute = LOWORD( lParam );
			int yPosAbsolute = HIWORD( lParam );
			g_Mouse->absMousePos[0] = xPosAbsolute;
			g_Mouse->absMousePos[1] = yPosAbsolute;
		}
	}
	return false;
}

struct WinProvider : public Provider
{
	uint32_t getNumVirtualPads() final
	{
		return 1;
	}

	auto setVirtualPadListener(uint32_t padIndex_, std::shared_ptr<VPadListener> const& listener_) -> void final
	{
		impl.listener = listener_;
	}

	auto getVirtualPadType(uint32_t padIndex_) -> VirtualPadType final
	{
		assert(padIndex_ < getNumVirtualPads());
		return VirtualPadType::KeyboardMouse;
	}
	auto update(double deltaT_) -> void final
	{
		impl.update();
	}

	KeyboardMouseListenerImpl impl;
};

auto Provider::WinCreateProvider() -> std::unique_ptr<Provider>
{
	return std::move(std::make_unique<WinProvider>());
}

} // end namespace

