#pragma once
#ifndef WYRD_INPUT_KEYBOARDMOUSELISTENERIMPL_H
#define WYRD_INPUT_KEYBOARDMOUSELISTENERIMPL_H

#include "core/core.h"
#include "input/vpadlistener.h"
namespace Input {

struct KeyboardMouseListenerImpl
{
	KeyboardMouseListenerImpl();
	~KeyboardMouseListenerImpl();

	auto update() -> void;
	std::shared_ptr<Input::VPadListener> listener;
};

}
#endif //WYRD_KEYBOARDMOUSELISTENERIMPL_H
