#pragma once
#ifndef WYRD_SHELL_MACSHELL_H
#define WYRD_SHELL_MACSHELL_H

#include "core/core.h"
#include "shell/consoleapp.h"

namespace Shell {

struct MacSpecific;

class MacShell : public ConsoleApp
{
public:
	MacShell();
	~MacShell() final;

	auto init(ShellConfig const& init_) -> bool final;
	auto update() -> bool final;
	auto ensureConsoleWindowsExists() -> void final;
	auto createPresentableWindow(PresentableWindowConfig const& config_) -> PresentableWindow* final;
	auto destroyPresentableWindow(PresentableWindow* window_) -> void final;

	auto getInputProvider(PresentableWindow* window_) -> std::unique_ptr<Input::Provider> final;

	MacSpecific* m;
};

}

#endif //WYRD_MACSHELL_H
