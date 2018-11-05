#include <resourcemanager/memstorage.h>
#include <render/resources.h>
#include <render/rendertarget.h>
#include "core/core.h"
#include "shell/interface.h"
#include "render/stable.h"
#include "render/encoder.h"
#include "resourcemanager/resourceman.h"

struct App
{
	App(Shell::ShellInterface& shell_) :
			shell(shell_) {}

	auto init() -> bool
	{
		using namespace std::string_literals;
		using namespace Core::bitmask;
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

		createResources();

		return okay;
	}

	auto finish() -> void
	{
		display.reset();
		device.reset();
		resourceManager.reset();
	}

	static constexpr std::string_view blankTex4x4Name = "mem$blankTex4x4";
	static constexpr std::string_view colourRT0Name = "mem$colourRT0";
	static constexpr std::string_view defaultRenderPassName = "mem$defaultRenderPass";
	static constexpr std::string_view defaultRenderTargetName = "mem$defaultRenderTarget";

	auto createResources() -> void
	{
		using namespace std::string_literals;
		using namespace Core::bitmask;
		using namespace Render;

		Texture blankTex4x4Def{
				{},
				TextureFlag::InitZero |
				TextureFlagFromUsage(Usage::ShaderRead),
				4, 4, 1, 1,
				1, 1,
				GenericTextureFormat::R8G8B8A8_UNORM,
				{}
		};
		resourceManager->placeInStorage(blankTex4x4Name, blankTex4x4Def);

		Texture colourRT0Def{
				{},
				TextureFlag::NoInit |
				TextureFlagFromUsage(Usage::RopRead | Usage::RopWrite),
				display->getWidth() / 2, display->getHeight() / 2, 1, 1,
				1, 1,
				GenericTextureFormat::R8G8B8A8_UNORM,
				{}
		};
		resourceManager->placeInStorage(colourRT0Name, colourRT0Def);

		RenderPass defaultRenderPassDef{
				{},
				1, // num targets
				{0, 0, 0}, // padds
				{0, 0, 0xFF, 0xFF}, // byte clear colour
				{
						{
								LoadOp::Clear,
								StoreOp::Store,
								GenericTextureFormat::R8G8B8A8_UNORM,
						}
				}
		};
		resourceManager->placeInStorage(defaultRenderPassName, defaultRenderPassDef);

		auto defaultRenderPassHandle = resourceManager->openResourceByName<RenderPass::Id>(defaultRenderPassName);
		auto colourRT0Handle = resourceManager->openResourceByName<Texture::Id>(colourRT0Name);

		RenderTarget defaultRenderTargetDef{
				{},
				defaultRenderPassHandle,
				{colourRT0Handle},
				{0, 0},
				{colourRT0Def.width, colourRT0Def.height}
		};
		resourceManager->placeInStorage(defaultRenderTargetName, defaultRenderTargetDef);
	}

	auto body() -> bool
	{
		using namespace Render;
		using namespace std::string_view_literals;

		// grab handles
		auto blankTexHandle = resourceManager->openResourceByName<Texture::Id>(blankTex4x4Name);
		auto colourRT0Handle = resourceManager->openResourceByName<Texture::Id>(colourRT0Name);
		auto defaultRenderPassHandle = resourceManager->openResourceByName<RenderPass::Id>(defaultRenderPassName);
		auto defaultRenderTargetHandle = resourceManager->openResourceByName<RenderTarget::Id>(defaultRenderTargetName);

		// acquire the resources
		auto blankTex = blankTexHandle.acquire<Texture>();
		auto colourRT0 = colourRT0Handle.acquire<Texture>();
		auto defaultRenderPass = defaultRenderPassHandle.acquire<RenderPass>();
		auto defaultRenderTarget = defaultRenderTargetHandle.acquire<RenderTarget>();

		auto renderQueue = device->getGeneralQueue();
		auto rEncoderPool = device->makeEncoderPool(true, CommandQueueFlavour::Render);

		Render::Encoder::Ptr encoder;
		do
		{
			rEncoderPool->reset();
			encoder = rEncoderPool->allocateEncoder(EncoderFlag::RenderEncoder);
			auto renderEncoder = encoder->asRenderEncoder();

			encoder->begin();
			renderEncoder->clearTexture(colourRT0, {1.0f, 0.0f, 0.0f, 1.0f});

			colourRT0->transitionToRenderTarget(encoder);
			renderEncoder->beginRenderPass(defaultRenderPass, defaultRenderTarget);
			renderEncoder->endRenderPass();
			colourRT0->transitionFromRenderTarget(encoder);

			encoder->end();

			renderQueue->enqueue(encoder);
			renderQueue->submit();
			renderQueue->stallTillIdle();
			device->houseKeepTick();
			display->present(colourRT0);
		} while(shell.update());

		return true;
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