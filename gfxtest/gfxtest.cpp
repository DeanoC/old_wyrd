#include "core/core.h"
#include "shell/interface.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "render/resources.h"
#include "render/rendertarget.h"
#include "render/shader.h"
#include "render/stable.h"
#include "render/encoder.h"
#include "render/pipeline.h"
#include "render/bindingtable.h"

static auto blankTex4x4Name = ResourceManager::ResourceNameView("mem$blankTex4x4");
static auto colourRT0Name = ResourceManager::ResourceNameView("mem$colourRT0");
static auto defaultRenderPassName = ResourceManager::ResourceNameView("mem$defaultRenderPass");
static auto defaultRenderTargetName = ResourceManager::ResourceNameView("mem$defaultRenderTarget");
static auto defaultBindingTableMemoryMapName = ResourceManager::ResourceNameView("mem$defaultBindingTableMemoryMap");
static auto defaultBindingTableName = ResourceManager::ResourceNameView("mem$defaultBindingTable");

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

	static constexpr std::string_view redOutFragmentShaderName = "mem$redIytFragmentShader";
	static constexpr std::string_view defaultRenderPipelineName = "mem$defaultRenderPipeline";

	auto createResources() -> void
	{
		using namespace std::string_literals;
		using namespace std::string_view_literals;
		using namespace Core::bitmask;
		using namespace Render;

		Texture::Create(
				resourceManager,
				blankTex4x4Name,
				TextureFlag::InitZero |
				TextureFlagFromUsage(Usage::ShaderRead | Usage::DMADst),
				4, 4, 1, 1,
				1, 1,
				GenericTextureFormat::R8G8B8A8_UNORM);

		Texture::Create(
				resourceManager,
				colourRT0Name,
				TextureFlag::NoInit |
				TextureFlagFromUsage(Usage::RopRead | Usage::RopWrite | Usage::DMASrc | Usage::DMADst),
				display->getWidth(), display->getHeight(), 1, 1,
				1, 1,
				GenericTextureFormat::R8G8B8A8_UNORM);

		auto defaultRenderPassHandle = RenderPass::Create(
				resourceManager,
				defaultRenderPassName,
				{{
						 LoadOp::Clear,
						 StoreOp::Store,
						 GenericTextureFormat::R8G8B8A8_UNORM,
				 }},
				{0, 0, 0xFF, 0xFF});

		RenderTarget::Create(
				resourceManager,
				defaultRenderTargetName,
				defaultRenderPassHandle,
				{resourceManager->openByName<Texture::Id>(colourRT0Name)},
				{0, 0},
				{display->getWidth(), display->getHeight()}
		);

		auto redOutFragmentShaderSourceHandle = ResourceManager::TextResource::Create(
				resourceManager,
				ResourceManager::ResourceNameView("mem$redOutFragmentShaderSource"sv),
				"[[vk::location(0)]] float4 main()\n"
				"{\n"
				"		return float4(1.0f, 0.0f, 0.0f, 1.0f);\n"
				"}\n");
		auto redOutFragmentShaderHandle = SPIRVShader::Compile(
				resourceManager,
				redOutFragmentShaderName,
				redOutFragmentShaderSourceHandle,
				ShaderSourceLanguage::HLSL,
				ShaderType::Fragment,
				0);

		RenderPipeline::Create(
				resourceManager,
				defaultRenderPipelineName,
				Topology::Triangles,
				0, // flags
				{}, // vertex
				{}, // tess control
				{}, // tess eval
				{}, // geometry
				redOutFragmentShaderHandle);
		BindingTableMemoryMap::Create(
				resourceManager,
				defaultBindingTableMemoryMapName,
				{});

		BindingTable::Create(
				resourceManager,
				defaultBindingTableName,
				{resourceManager->openByName<BindingTableMemoryMapId>(defaultBindingTableMemoryMapName)});
	}

	auto body() -> bool
	{
		using namespace Render;
		using namespace std::string_view_literals;

		// grab handles
		auto blankTexHandle = resourceManager->openByName<Texture::Id>(blankTex4x4Name);
		auto colourRT0Handle = resourceManager->openByName<Texture::Id>(colourRT0Name);
		auto defaultRenderPassHandle = resourceManager->openByName<RenderPass::Id>(defaultRenderPassName);
		auto defaultRenderTargetHandle = resourceManager->openByName<RenderTarget::Id>(defaultRenderTargetName);
		auto fragShaderHandle = resourceManager->openByName<SPIRVShader::Id>(redOutFragmentShaderName);

		// acquire the resources
		auto blankTex = blankTexHandle.acquire<Texture>();
		auto colourRT0 = colourRT0Handle.acquire<Texture>();
		auto defaultRenderPass = defaultRenderPassHandle.acquire<RenderPass>();
		auto defaultRenderTarget = defaultRenderTargetHandle.acquire<RenderTarget>();
		auto fragShader = fragShaderHandle.acquire<SPIRVShader>();

		auto renderQueue = device->getGeneralQueue();
		auto rEncoderPool = device->makeEncoderPool(true, CommandQueueFlavour::Render);

		Render::Encoder::Ptr encoder;
		do
		{
			rEncoderPool->reset();
			encoder = rEncoderPool->allocateEncoder(EncoderFlag::RenderEncoder);
			auto renderEncoder = encoder->asRenderEncoder();

			encoder->begin();
			colourRT0->transitionToDMADest(encoder);
			renderEncoder->clearTexture(colourRT0, {1.0f, 0.0f, 0.0f, 1.0f});

			colourRT0->transitionToRenderTarget(encoder);
			renderEncoder->beginRenderPass(defaultRenderPass, defaultRenderTarget);
			renderEncoder->endRenderPass();
			colourRT0->transitionToDMASrc(encoder);
			
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