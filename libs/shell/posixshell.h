#pragma once
#ifndef WYRD_SHELL_POSIXSHELL_H
#define WYRD_SHELL_POSIXSHELL_H

#include "core/core.h"
#include "shell/consoleapp.h"

namespace Shell {
class PosixShell : public ConsoleApp
{
	PosixShell();
	~PosixShell() final;

	auto init(ShellConfig const& init_) -> bool final;
	auto update() -> bool final;
	auto ensureConsoleWindowsExists() -> void final;
	auto createPresentableWindow(PresentableWindowConfig const& config_) -> PresentableWindow* final;

};

}

#endif //WYRD_POSIXSHELL_H
