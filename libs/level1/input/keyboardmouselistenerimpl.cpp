#include "core/core.h"
#include "input/keyboardmouselistenerimpl.h"
#include "input/keyboard.h"
#include "input/mouse.h"

namespace Input
{

KeyboardMouseListenerImpl::KeyboardMouseListenerImpl()
{
	assert(Input::g_Keyboard != nullptr);
	assert(Input::g_Mouse != nullptr);
}

KeyboardMouseListenerImpl::~KeyboardMouseListenerImpl()
{
	listener.reset();
}

auto KeyboardMouseListenerImpl::update() -> void
{
	if(!listener) return;

	// simulate pad input on keyboard and mouse
	auto k = g_Keyboard;
	auto m = g_Mouse;
	float lx = 0.f, ly = 0.f;
	float rx = 0, ry = 0;
	float fbutton = 0.0f;
	float sbutton = 0.0f;
	float tbutton = 0.0f;


	if(k && !k->isInputConsumed())
	{
		lx += k->keyDown(Key::KT_A) ? -1.0f : 0.0f;
		lx += k->keyDown(Key::KT_D) ?  1.0f : 0.0f;
		ly += k->keyDown(Key::KT_W) ?  1.0f : 0.0f;
		ly += k->keyDown(Key::KT_S) ? -1.0f : 0.0f;

		// simulate right stick and mouse button only if no mouse
		if(!m)
		{
			rx += k->keyDown(Key::KT_LEFT) ? -0.005f : 0.f;
			rx += k->keyDown(Key::KT_RIGHT) ? 0.005f : 0.f;
			ry += k->keyDown(Key::KT_UP) ? 0.005f : 0.f;
			ry += k->keyDown(Key::KT_DOWN) ? -0.005f : 0.f;
			fbutton += k->keyDown(Key::KT_RCONTROL) ? 1.0f : 0.0f;
			sbutton += k->keyDown(Key::KT_RSHIFT) ? 1.0f : 0.0f;
		}

	}

	if(m && !m->isInputConsumed())
	{
		fbutton += m->buttonDown(MouseButton::Left) ? 1.0f : 0.0f;
		sbutton += m->buttonDown(MouseButton::Right) ? 1.0f : 0.0f;
//		rx += m->getRelX();
//		ry += m->getRelY();
	}

#define NOT_ZERO_AXIS(v, ax) if( fabsf(v) > 1e-5f ) listener->axisMovement( VPadAxisId:: ax, v );
#define NOT_ZERO_BUTTON(v, ax) if( fabsf(v) > 1e-5f ) listener->button( VPadButtonId:: ax, v );

	NOT_ZERO_AXIS(lx, LX)
	NOT_ZERO_AXIS(ly, LY)
	NOT_ZERO_AXIS(rx, RX)
	NOT_ZERO_AXIS(ry, RY)
	NOT_ZERO_BUTTON(fbutton, First)
	NOT_ZERO_BUTTON(sbutton, Second)
	NOT_ZERO_BUTTON(tbutton, Third)

#undef NOT_ZERO_AXIS
#undef NOT_ZERO_BUTTON

}


}