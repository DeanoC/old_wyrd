#define LOGURU_IMPLEMENTATION 1
#define CX_ERROR_IMPLEMENTATION 1
#include "core/core.h"
#include "shell/shell.h"
#include "render/stable.h"

int Main(Shell::ShellInterface& shell_)
{
	bool okay = shell_.init({
						"Gfx Test",
						true,
						true,
				});
	if(!okay) return 10;

	// use a presentable and descrete if possible else integrated presentable
	Render::Stable* gpuStable = shell_.getGpuStable();

	uint32_t pickedGpuIndex = ~0;
	for(auto gpuIndex = 0u; gpuIndex < gpuStable->getGpuCount(); ++gpuIndex)
	{
		if(gpuStable->canGpuPresent(gpuIndex))
		{
			pickedGpuIndex = gpuIndex;
			if(gpuStable->isGpuLowPower( gpuIndex ) == false)
			{
				break;
			}
		}
	}
	if(pickedGpuIndex == ~0) return 10;
	Render::DeviceConfig config = {
			true, 	// presentable
			true, 	// renderer
			true, 	// compute
			{},		// no extensions
			1280, 720, // 720p
			false,		// no hdr
	};
	auto device = gpuStable->createGpuDevice(pickedGpuIndex, config);
	if(!device) return 10;

	return 0;
}