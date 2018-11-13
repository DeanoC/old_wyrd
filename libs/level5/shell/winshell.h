#pragma once
#ifndef WYRD_SHELL_WINSHELL_H
#define WYRD_SHELL_WINSHELL_H

#include "core/core.h"
#include "shell/consoleapp.h"

namespace Shell {
struct WindowsWinMainCreationParams;
struct WindowsSpecific;

struct WinShell : public ConsoleApp
{
	WinShell(WindowsWinMainCreationParams*);
	~WinShell() final;

	auto init(ShellConfig const& init_) -> bool final;
	auto update() -> bool final;
	auto ensureConsoleWindowsExists() -> void final;
	auto createPresentableWindow(PresentableWindowConfig const& config_) -> PresentableWindow* final;

	WindowsSpecific* w;
};


}

#endif //WYRD_SHELL_WINSHELL_H
