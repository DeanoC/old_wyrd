#pragma once
#ifndef WYRD_INPUT_INPUT_H
#define WYRD_INPUT_INPUT_H

#include <core/core.h>

namespace Input {

// generates a Key enum class from the platform virtual keys

// each key has a label KT_x (i.e. A = KT_A
#if PLATFORM == WINDOWS
#undef DELETE
#define VIRTUALKEY_KEY_MAP2(c, w, m) KT_##c = VK_##w,
#define VIRTUALKEY_KEY_MAP_WINONLY(c, w) KT_##c = VK_##w,
#define VIRTUALKEY_KEY_MAP(c) VIRTUALKEY_KEY_MAP2(c, c, c)

#define NORMALKEY_KEY_MAP(c, d) KT_##c = d,
#elif PLATFORM == APPLE_MAC

#include "input/osx_keycodes.h"

#define VIRTUALKEY_KEY_MAP2(c, w, m) KT_##c = (uint16_t)MacVirtualKeyCode::KVK_ ##m,
#define VIRTUALKEY_KEY_MAP_WINONLY(c, w)
#define VIRTUALKEY_KEY_MAP(c) VIRTUALKEY_KEY_MAP2(c, c, c)

#define NORMALKEY_KEY_MAP(c, d) KT_##c = (uint16_t)MacVirtualKeyCode::KVK_ANSI_ ##c,
#endif

#include "input/keylist.h"

enum class MouseButton : uint8_t
{
	Left,
	Middle,
	Right,
};
static constexpr uint32_t MaxMouseButtons = ((uint32_t) MouseButton::Right)+1u;

}

#endif //WYRD_INPUT_INPUT_H
