#include <resourcemanager/memstorage.h>
#include <render/resourcehandlers.h>
#include "core/core.h"
#include "shell/interface.h"
#include "render/stable.h"
#include "render/encoder.h"
#include "resourcemanager/resourceman.h"

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

	Shell::PresentableWindowConfig windowConfig;
	windowConfig.width = 1280;
	windowConfig.height = 720;
	windowConfig.windowName = "GfxTest"s;
	windowConfig.fullscreen = false;
	auto window = shell_.createPresentableWindow(windowConfig);
	Render::DeviceConfig config = {
			true, 	// presentable
			true, 	// renderer
			true, 	// compute
			{},		// no extensions
			window,
			windowConfig.width, windowConfig.height, // 720p
			false,		// no hdr
	};
	auto device = gpuStable->createGpuDevice(pickedGpuIndex, config, resourceManager);
	if(!device) return 10;

	auto display = device->getDisplay();
	Render::Texture blankTexture256x256Def
			{
					{},
					TextureFlag::InitZero,
					256, 256, 1, 1,
					1, 1, GenericTextureFormat::R8G8B8A8_UNORM,
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

	auto renderQueue = device->getGeneralQueue();
	auto rEncoderPool = device->makeEncoderPool(true, CommandQueueFlavour::Render);

	Render::Encoder::Ptr encoder;
	do
	{
		rEncoderPool->reset();
		encoder = rEncoderPool->allocateEncoder(EncoderFlag::RenderEncoder);

		encoder->begin();

		auto renderEncoder = encoder->asRenderEncoder();
		renderEncoder->beginRenderPass();
		renderEncoder->endRenderPass();
		encoder->end();

		renderQueue->enqueue(encoder);
		renderQueue->submit();
		//		renderQueue->stallTillIdle();
		device->houseKeepTick();
		display->present(blankTex);
	} while(shell_.update());

	return 0;
}