
#include "core/core.h"
#include "shell/interface.h"
#include "render/stable.h"

int Main(Shell::ShellInterface& shell_)
{
	bool okay = shell_.init({
									"Vulkan Info",
									true,
									false,
							});
	if(!okay) return 10;

	Render::Stable* gpuStable = shell_.getGpuStable();

	uint32_t pickedGpuIndex = ~0;
	for(auto gpuIndex = 0u; gpuIndex < gpuStable->getGpuCount(); ++gpuIndex)
	{
		if(gpuStable->canGpuPresent(gpuIndex))
		{
			pickedGpuIndex = gpuIndex;
			if(gpuStable->isGpuLowPower(gpuIndex) == false)
			{
				break;
			}
		}
	}
	if(pickedGpuIndex == ~0) return 10;
	Render::DeviceConfig config = {
			false,    // presentable
			false,    // renderer
			true,    // compute
			{},        // no extensions
	};
	//	auto device = gpuStable->createGpuDevice(pickedGpuIndex, config);
	//	if(!device) return 10;

	return 0;
}