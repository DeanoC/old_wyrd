#pragma once
#ifndef WYRD_SHELL_INTERFACE_H
#define WYRD_SHELL_INTERFACE_H

#include "core/core.h"
#include <vector>
#include <string>

namespace Render {
struct Stable;
}

// define WYRD_SHELL_CONSOLE_APP to get a console app else expect a gui (windows only?)
// define WYRD_SHELL_NOSHELL to supply your own initial main and shell
// otherwise shell will handle main/WinMain etc. and call your Main(Shell& shell_)

// normal usage for Shell, is that it calls your int Main(Shell& shell_)
// you then set the app name and other properties (compute, render, etc.)
// call init, Shell will return true if everything is okay
// call update regularly whilst running, false indiacated OS has asked app to close
// Main returns 0 == okay and Shell cleans up and exits
namespace Shell
{
struct ShellConfig
{
	std::string appName;

	// render section if compute or render is required, render(s) will be setup
	bool gpuComputeRequired;
	bool gpuRenderRequired;
	std::vector<std::string> gpuBackendSpecific; // e.g. Vulkan extensions required

};

struct ShellInterface
{
	virtual ~ShellInterface() {}

	virtual auto getArguments() const -> std::vector<std::string> const& = 0;

	virtual auto init(ShellConfig const& init_) -> bool = 0;

	virtual auto update() -> bool = 0;

	virtual auto getShellConfig() -> ShellConfig const& = 0;

	virtual auto gpusInitialised() -> bool = 0;

	virtual auto getGpuStable() -> Render::Stable* = 0;
};

} // end shell namespace

extern int Main(Shell::ShellInterface&);

#endif //WYRD_SHELL_INTERFACE_H