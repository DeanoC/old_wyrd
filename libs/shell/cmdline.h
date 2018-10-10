#pragma once
#ifndef WYRD_SHELL_CMDLINE_H
#define WYRD_SHELL_CMDLINE_H

#include <render/stable.h>
#include "core/core.h"
#include "shell/shell.h"

namespace Shell {

class CmdLine : public ShellInterface
{
public:
	~CmdLine();

	CmdLine(int argc_, char const*const* argv_);

	auto getArguments() const -> std::vector<std::string> const& final;

	auto init(ShellConfig const& init_) -> bool final;

	auto update() -> bool final;

	auto getShellConfig() -> ShellConfig const& final { return userInit; };

	auto gpusInitialised() -> bool final { return userInit.gpuRenderRequired || userInit.gpuComputeRequired;};

	auto getGpuStable() -> Render::Stable* final { return gpuStable.get(); };

public:
	ShellConfig userInit;

	std::vector<std::string> arguments;
	std::unique_ptr<Render::Stable> gpuStable;
};

}

#endif //WYRD_SHELL_CMDLINE_H
