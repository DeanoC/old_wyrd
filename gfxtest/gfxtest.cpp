#define LOGURU_IMPLEMENTATION 1
#define CX_ERROR_IMPLEMENTATION 1

#include "core/core.h"
#include "shell/shell.h"
#include "render/stable.h"
#include "render/commandqueue.h"
#include "render/encoder.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/memstorage.h"
#include "render/resourcehandlers.h"

int Main(Shell::ShellInterface& shell_)
{
	using namespace Render;
	using namespace std::string_literals;
	using namespace std::string_view_literals;

	bool okay = shell_.init({
						"Gfx Test",
						true,
						true,
				});
	if(!okay) return 10;

	auto resourceManager = ResourceManager::ResourceMan::Create();
	auto memstorage = std::make_shared<ResourceManager::MemStorage>();
	resourceManager->registerStorageHandler(memstorage);
	Render::RegisterResourceHandlers(*resourceManager);

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
	auto device = gpuStable->createGpuDevice(pickedGpuIndex, config, resourceManager);
	if(!device) return 10;

	auto display = device->getDisplay();
	Render::Texture blankTexture256x256Def
			{
					{},
					Texture::InitZeroFlag,
					256, 256, 1, 1,
					1, 1, GenericTextureFormat::R8G8B8A8_UINT,
					{}
			};
	memstorage->addMemory("BlankTexture_256x256"s,
						  Texture::Id,
						  Texture::MajorVersion,
						  Texture::MinorVersion,
						  &blankTexture256x256Def, sizeof(Render::Texture));

	ResourceManager::ResourceName blankTexName(memstorage->getPrefix(), "BlankTexture_256x256"sv);

	auto blankTexHandle = resourceManager->openResourceByName<Texture::Id>(blankTexName.getView());

	auto blankTex = blankTexHandle.acquire<Texture>();

	auto renderQueue = device->getMainRenderQueue();
	auto rEncoderPool = device->makeEncoderPool(true, CommandQueue::RenderFlavour);

	Render::Encoder::Ptr encoder;
	do
	{
		/*		rEncoderPool->reset();
				encoder = rEncoderPool->allocateEncoder(CommandQueue::RenderFlavour);

				encoder->begin();

				auto renderEncoder = encoder->asRenderEncoder();
				renderEncoder->beginRenderPass();
				renderEncoder->endRenderPass();
				encoder->end();

				renderQueue->enqueue(encoder);
				renderQueue->submit();
		//		renderQueue->stallTillIdle();*/
	} while(display->present(blankTex));

	return 0;
}