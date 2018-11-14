#pragma once
#ifndef WYRD_INPUT_INPUT_H
#define WYRD_INPUT_INPUT_H

#include <core/core.h>

namespace Input {

// generates a Key enum class from the platform virtual keys

// each key has a label KT_x (i.e. A = KT_A
#if PLATFORM == WINDOWS
#undef DELETE
#define VIRTUALKEY_KEY_MAP2(x, y) KT_##x = VK_##y,
#define VIRTUALKEY_KEY_MAP(x) VIRTUALKEY_KEY_MAP2(x,x)
#define NORMALKEY_KEY_MAP(c, d) KT_##c = d,
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
