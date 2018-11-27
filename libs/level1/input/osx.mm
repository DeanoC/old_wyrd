#include "core/core.h"
#include "input/keylist.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/provider.h"
#include "keyboardmouselistenerimpl.h"

#include <thread>
#import <Cocoa/Cocoa.h>

namespace Input {


/*
bool Keyboard::MacProcessEvents(NSEvent* event, NSView* view)
{
	// FIXME: All the key handling is wrong and broken. Refer to GLFW's cocoa_init.mm and cocoa_window.mm.
	if(event.type == NSEventTypeKeyDown)
	{
		NSString* str = [event characters];
		int len = (int) [str length];
		for(int i = 0; i < len; i++)
		{
			int c = [str characterAtIndex:i];
			if(c < 0xF700 && !io.KeyCtrl)
				io.AddInputCharacter((unsigned short) c);

			// We must reset in case we're pressing a sequence of special keys while keeping the command pressed
			int key = mapCharacterToKey(c);
			if(key != -1 && key < 256 && !io.KeyCtrl)
				resetKeys();
			if(key != -1)
				io.KeysDown[key] = true;
		}
		return io.WantCaptureKeyboard;
	}

	if(event.type == NSEventTypeKeyUp)
	{
		NSString* str = [event characters];
		int len = (int) [str length];
		for(int i = 0; i < len; i++)
		{
			int c = [str characterAtIndex:i];
			int key = mapCharacterToKey(c);
			if(key != -1)
				io.KeysDown[key] = false;
		}
		return io.WantCaptureKeyboard;
	}


	if(event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown ||
	   event.type == NSEventTypeOtherMouseDown)
	{
		int button = (int) [event buttonNumber];
		if(button >= 0 && button < IM_ARRAYSIZE(io.MouseDown))
			io.MouseDown[button] = true;
		return io.WantCaptureMouse;
	}

	if(event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp ||
	   event.type == NSEventTypeOtherMouseUp)
	{
		int button = (int) [event buttonNumber];
		if(button >= 0 && button < IM_ARRAYSIZE(io.MouseDown))
			io.MouseDown[button] = false;
		return io.WantCaptureMouse;
	}

	if(event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged)
	{
		NSPoint mousePoint = event.locationInWindow;
		mousePoint = [view convertPoint:mousePoint fromView:nil];
		mousePoint = NSMakePoint(mousePoint.x, view.bounds.size.height - mousePoint.y);
		io.MousePos = ImVec2(mousePoint.x, mousePoint.y);
	}

	if(event.type == NSEventTypeScrollWheel)
	{
		double wheel_dx = 0.0;
		double wheel_dy = 0.0;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
		if(floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
		{
			wheel_dx = [event scrollingDeltaX];
			wheel_dy = [event scrollingDeltaY];
			if([event hasPreciseScrollingDeltas])
			{
				wheel_dx *= 0.1;
				wheel_dy *= 0.1;
			}
		} else
#endif
		{
			wheel_dx = [event deltaX];
			wheel_dy = [event deltaY];
		}

		if(fabs(wheel_dx) > 0.0)
			io.MouseWheelH += wheel_dx * 0.1f;
		if(fabs(wheel_dy) > 0.0)
			io.MouseWheel += wheel_dy * 0.1f;
		return io.WantCaptureMouse;
	}

	// FIXME: All the key handling is wrong and broken. Refer to GLFW's cocoa_init.mm and cocoa_window.mm.
	if(event.type == NSEventTypeKeyDown)
	{
		NSString* str = [event characters];
		int len = (int) [str length];
		for(int i = 0; i < len; i++)
		{
			int c = [str characterAtIndex:i];
			if(c < 0xF700 && !io.KeyCtrl)
				io.AddInputCharacter((unsigned short) c);

			// We must reset in case we're pressing a sequence of special keys while keeping the command pressed
			int key = mapCharacterToKey(c);
			if(key != -1 && key < 256 && !io.KeyCtrl)
				resetKeys();
			if(key != -1)
				io.KeysDown[key] = true;
		}
		return io.WantCaptureKeyboard;
	}

	if(event.type == NSEventTypeKeyUp)
	{
		NSString* str = [event characters];
		int len = (int) [str length];
		for(int i = 0; i < len; i++)
		{
			int c = [str characterAtIndex:i];
			int key = mapCharacterToKey(c);
			if(key != -1)
				io.KeysDown[key] = false;
		}
		return io.WantCaptureKeyboard;
	}

	if(event.type == NSEventTypeFlagsChanged)
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned int flags = [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;

		bool oldKeyCtrl = io.KeyCtrl;
		bool oldKeyShift = io.KeyShift;
		bool oldKeyAlt = io.KeyAlt;
		bool oldKeySuper = io.KeySuper;
		io.KeyCtrl = flags & NSEventModifierFlagControl;
		io.KeyShift = flags & NSEventModifierFlagShift;
		io.KeyAlt = flags & NSEventModifierFlagOption;
		io.KeySuper = flags & NSEventModifierFlagCommand;

		// We must reset them as we will not receive any keyUp event if they where pressed with a modifier
		if((oldKeyShift && !io.KeyShift) || (oldKeyCtrl && !io.KeyCtrl) || (oldKeyAlt && !io.KeyAlt) ||
		   (oldKeySuper && !io.KeySuper))
			resetKeys();
		return io.WantCaptureKeyboard;
	}

	return false;
}

bool Keyboard::WinProcessMessages(uint32_t message, uint16_t wParam, uint32_t lParam)
{
	if(g_Keyboard == nullptr) return false;

	TODO
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			return 0;

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
			g_Keyboard->keyDownBitMap[(uint16_t) key] = true;
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
			g_Keyboard->keyDownBitMap[(uint16_t) key] = false;
			g_Keyboard->keyDataState[(uint16_t) key] = 0;
		}
		return true;
	}
	return false;
}
*/

struct MacProvider : public Provider
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

auto Provider::MacCreateProvider() -> std::unique_ptr<Provider>
{
	return std::move(std::make_unique<MacProvider>());
}

} // end namespace

