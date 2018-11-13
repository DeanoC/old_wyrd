#pragma once
#ifndef WYRD_SHELL_CMDLINE_H
#define WYRD_SHELL_CMDLINE_H

#include <render/stable.h>
#include "core/core.h"
#include "shell/interface.h"

namespace Shell {

struct ConsoleApp : public ShellInterface
{
public:
	ConsoleApp();
	~ConsoleApp() override;

	auto sysInit(int argc_, char** argv_) -> void;

	auto getArguments() const -> std::vector<std::string> const& final;
	auto getShellConfig() -> ShellConfig const& final { return userInit; };
	auto gpusInitialised() -> bool final { return userInit.gpuRenderRequired || userInit.gpuComputeRequired;};
	auto getGpuStable() -> Render::Stable* final { return gpuStable.get(); };

	auto ensureConsoleWindowsExists() -> void override {};

	auto init(ShellConfig const& init_) -> bool override;
	auto update() -> bool override;

	auto createPresentableWindow(
			PresentableWindowConfig const& config_) -> PresentableWindow* override { return nullptr; };

	ShellConfig userInit;

	std::vector<std::string> arguments;
	std::unique_ptr<Render::Stable> gpuStable;
};

}

#endif //WYRD_SHELL_CMDLINE_H
