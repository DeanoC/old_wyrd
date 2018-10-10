#include "core/core.h"
#include "shell/cmdline.h"
#include "vulkan/system.h"

#if PLATFORM == WINDOWS
#else
#endif

namespace Shell {
CmdLine::~CmdLine()
{
	if(userInit.gpuComputeRequired || userInit.gpuRenderRequired)
	{
		gpuStable->Finish();
		gpuStable.reset();
	}

}

CmdLine::CmdLine( 	int argc_,
					char const*const* argv_)
{
	arguments.resize(argc_);
	for(auto i = 0u; i < argc_; ++i)
	{
		arguments[i] = std::string(argv_[i]);
	}
}

auto CmdLine::getArguments() const -> std::vector<std::string> const&
{
	return arguments;
}

auto CmdLine::init(ShellConfig const& init_) -> bool
{
	userInit = init_;

	if(gpusInitialised())
	{
		// at the moment only vulkan supported
		gpuStable = std::make_unique<Vulkan::System>();
		bool okay = gpuStable->Init( userInit.appName, userInit.gpuBackendSpecific );
		if(okay == false)
		{
			userInit.gpuRenderRequired = false;
			userInit.gpuComputeRequired = false;
			return false;
		}
	}

	return true;
}

auto CmdLine::update() -> bool
{
	return false;
}

} // end namespace