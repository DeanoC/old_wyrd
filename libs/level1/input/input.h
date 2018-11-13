#pragma once
#ifndef WYRD_INPUT_INPUT_H
#define WYRD_INPUT_INPUT_H

#include <core/core.h>

namespace Input {

// each key has a label KT_x (i.e. A = KT_A
enum class Key : uint16_t
{
#if PLATFORM == WINDOWS
#define VIRTUALKEY_KEY_MAP2(x, y) KT_##x = VK_##y,
#define VIRTUALKEY_KEY_MAP(x) VIRTUALKEY_KEY_MAP2(x,x)
#define NORMALKEY_KEY_MAP(c, d) KT_##c = d,
#endif

#include "input/keylist.h"

};

}

#endif //WYRD_INPUT_INPUT_H
