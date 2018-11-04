#include <resourcemanager/memstorage.h>
#include <render/resourcehandlers.h>
#include "core/core.h"
#include "shell/interface.h"
#include "render/stable.h"
#include "render/encoder.h"
#include "resourcemanager/resourceman.h"

struct App
{
	App(Shell::ShellInterface& shell_) :
			shell(shell_) {}

	static constexpr std::string_view blankTex4x4Name = "mem$blankTex4x4";

	auto init() -> bool
	{
		using namespace std::string_literals;
		using namespace Render;

		Shell::ShellConfig shellConfig{};
		shellConfig.appName = "Gfx Test";
		shellConfig.wantConsoleOutput = false;
		shellConfig.gpuComputeRequired = true;
		shellConfig.gpuRenderRequired = true;
		bool okay = shell.init(shellConfig);
		if(!okay) return false;

		resourceManager = ResourceManager::ResourceMan::Create();
		auto memstorage = std::make_shared<ResourceManager::MemStorage>();
		resourceManager->registerStorageHandler(memstorage);

		RegisterResourceHandlers(*resourceManager);

		// use a presentable and descrete if possible else integrated presentable
		Stable* gpuStable = shell.getGpuStable();

		uint32_t pickedGpuIndex = ~0;
		for(auto gpuIndex = 0u; gpuIndex < gpuStable->getGpuCount(); ++gpuIndex)
		{
			if(gpuStable->canGpuPresent(gpuIndex))
			{
				pickedGpuIndex = gpuIndex;
				if(gpuStable->isGpuLowPower(gpuIndex) == false) break;
			}
		}
		if(pickedGpuIndex == ~0) return false;

		Shell::PresentableWindowConfig windowConfig;
		windowConfig.width = 1280;
		windowConfig.height = 720;
		windowConfig.windowName = "GfxTest"s;
		windowConfig.fullscreen = false;
		auto window = shell.createPresentableWindow(windowConfig);
		DeviceConfig config = {
				true,    // presentable
				true,    // renderer
				true,    // compute
				{},        // no extensions
				window,
				windowConfig.width, windowConfig.height, // 720p
				false,        // no hdr
		};
		device = gpuStable->createGpuDevice(pickedGpuIndex, config, resourceManager);
		if(!device) return false;

		display = device->getDisplay();

		Texture blankTex4x4Def{
				{},
				TextureFlag::InitZero,
				4, 4, 1, 1,
				1, 1, Render::GenericTextureFormat::R8G8B8A8_UNORM,
				{}
		};
		Texture::PlaceInStorage(blankTex4x4Name, blankTex4x4Def, resourceManager);

		return okay;
	}

	auto body() -> bool
	{
		using namespace Render;
		using namespace std::string_view_literals;

		auto blankTexHandle = resourceManager->openResourceByName<Texture::Id>(blankTex4x4Name);
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
		} while(shell.update());

		return true;
	}

	auto finish() -> void
	{
		display.reset();
		device.reset();
		resourceManager.reset();
	}

	ResourceManager::ResourceMan::Ptr resourceManager;
	Shell::ShellInterface& shell;
	Render::Device::Ptr device;
	Render::Display::Ptr display;
};

int Main(Shell::ShellInterface& shell_)
{
	using namespace Render;
	using namespace std::string_literals;
	using namespace std::string_view_literals;

	App app(shell_);

	if(!app.init()) return 10;

	bool okay = app.body();

	app.finish();
	if(!okay) return 10;
	else return 0;
}