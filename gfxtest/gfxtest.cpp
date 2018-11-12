#include <input/keyboard.h>
#include "core/core.h"
#include "shell/interface.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "render/buffer.h"
#include "render/resources.h"
#include "render/rendertarget.h"
#include "render/shader.h"
#include "render/stable.h"
#include "render/encoder.h"
#include "render/pipeline.h"
#include "render/bindingtable.h"
#include "render/viewport.h"
#include "render/vertexinput.h"
#include "render/ropblender.h"
#include "render/rasterisationstate.h"
#include "midrender/stocks.h"

static auto defaultBindingTableMemoryMapName = ResourceManager::ResourceNameView("mem$defaultBindingTableMemoryMap");
static auto defaultBindingTableName = ResourceManager::ResourceNameView("mem$defaultBindingTable");
static auto defaultRenderPipelineName = ResourceManager::ResourceNameView("mem$defaultRenderPipeline");

static auto basicTriVertexBufferName = ResourceManager::ResourceNameView("mem$basicTriVertexBuffer");
static auto basicTriVertexBuffer2Name = ResourceManager::ResourceNameView("mem$basicTriVertexBuffer2");
static auto basicTriIndexBufferName = ResourceManager::ResourceNameView("mem$basicTriIndexBuffer");
static auto pushColourFragmentShaderName = ResourceManager::ResourceNameView("mem$pushColourOutFragmentShader");

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
		windowConfig.directInput = true;
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

	auto createResources() -> void
	{
		using namespace std::string_literals;
		using namespace std::string_view_literals;
		using namespace Core::bitmask;
		using namespace Render;
		using namespace ResourceManager;
		using namespace MidRender;

		// alias resourceManager name (why can't I use 'using' C++ standard body!)
		auto& rm = resourceManager;
		MidRender::Stocks::InitBasics(rm);
		MidRender::Stocks::InitSimpleForwardRenderer(rm,
													 display->getWidth(),
													 display->getHeight());

		Buffer::Create<float>(rm,
							  basicTriVertexBufferName,
							  Buffer::FromUsage(Usage::VertexRead | Usage::DMADst),
							  {
									  0.0f, 1.0f, 0.5f,
									  1.0f, 1.0f, 0.5f,
									  1.0f, 0.0f, 0.5f
							  });

		Buffer::Create<float>(rm,
							  basicTriVertexBuffer2Name,
							  Buffer::FromUsage(Usage::VertexRead | Usage::DMADst),
							  {
									  0.0f, 0.0f, 0.5f,
									  1.0f, 1.0f, 0.5f,
									  0.0f, 1.0f, 0.5f
							  });

		Buffer::Create<uint16_t>(rm,
								 basicTriIndexBufferName,
								 Buffer::FromUsage(Usage::IndexRead | Usage::DMADst),
								 {
										 0, 1, 2
								 });

		auto const hlslPushColourShaderSourceName = ResourceNameView("mem$hlslPushColourOutFragmentShaderSource"sv);
		auto const glslPushColourShaderSourceName = ResourceNameView("mem$glslPushColourOutFragmentShaderSource"sv);

		TextResource::Create(rm,
							 hlslPushColourShaderSourceName,
							 "[[vk::push_constant]] cbuffer S { [[vk::offset(64)]] float4 colour; };\n"
							 "[[vk::location(0)]] float4 main()\n"
							 "{\n"
							 "		return colour;\n"
							 "}\n");

		TextResource::Create(rm,
							 glslPushColourShaderSourceName,
							 "#extension GL_ARB_separate_shader_objects : enable\n"
							 "#extension GL_ARB_shading_language_420pack : enable\n"
							 "layout(push_constant) uniform pushConstants_t { layout(offset = 64) vec4 colour; } pushConstants;\n"
							 "layout (location = 0) out vec4 outColor;\n"
							 "void main() {\n"
							 "   outColor = pushConstants.colour;\n"
							 "}\n");

		SPIRVShader::Compile(rm,
							 pushColourFragmentShaderName,
							 {rm->openByName<TextResourceId>(glslPushColourShaderSourceName)},
							 ShaderSourceLanguage::GLSL,
							 ShaderType::Fragment,
							 0);

		BindingTableMemoryMap::Create(rm,
									  defaultBindingTableMemoryMapName,
									  {}
		);

		BindingTable::Create(rm,
							 defaultBindingTableName,
							 {rm->openByName<BindingTableMemoryMapId>(defaultBindingTableMemoryMapName)});


		RenderPipeline::Create(rm,
							   defaultRenderPipelineName,
							   Topology::Triangles,
							   RenderPipelineFlags::None,
							   {
									   rm->openByName<BindingTableMemoryMapId>(defaultBindingTableMemoryMapName)
							   },
							   {
									   {0,  sizeof(float) * 16, ShaderType::Vertex}, // world matrix
									   {64, sizeof(float) * 4,  ShaderType::Fragment}, // colour
							   },
							   {
									   rm->openByName<SPIRVShaderId>(Stock::passthroughVertexShader),
									   rm->openByName<SPIRVShaderId>(pushColourFragmentShaderName),
							   },
							   rm->openByName<RasterisationStateId>(Stock::simpleForwardRendererDefaultPipeline),
							   rm->openByName<RenderPassId>(Stock::simpleForwardRendererRenderPass),
							   rm->openByName<ROPBlenderId>(Stock::defaultROPBlender),
							   rm->openByName<ViewportId>(Stock::simpleForwardRendererViewport),
							   rm->openByName<VertexInputId>(Stock::positionOnlyVertexInput)
		);
	}

	auto body() -> bool
	{
		using namespace Render;
		using namespace MidRender;;
		using namespace std::string_view_literals;

		// acquire the resources by name
		auto& rm = resourceManager;
		auto colourRT0 = rm->acquireByName<Texture>(Stock::simpleForwardRendererColourRT);
		auto renderPass = rm->acquireByName<RenderPass>(Stock::simpleForwardRendererRenderPass);
		auto renderTarget = rm->acquireByName<RenderTarget>(Stock::simpleForwardRendererRenderTarget);
		auto renderPipeline = rm->acquireByName<RenderPipeline>(defaultRenderPipelineName);

		auto vBuffer = rm->acquireByName<Buffer>(basicTriVertexBufferName);
		auto vBuffer2 = rm->acquireByName<Buffer>(basicTriVertexBuffer2Name);
		auto iBuffer = rm->acquireByName<Buffer>(basicTriIndexBufferName);

		auto renderQueue = device->getGeneralQueue();
		auto rEncoderPool = device->makeEncoderPool(true, CommandQueueFlavour::Render);

		std::array<float, 4> const red{1.0f, 0.0f, 0.0f, 1.0f};
		std::array<float, 4> const white{1.0f, 1.0f, 1.0f, 1.0f};

		using namespace Core::bitmask;

		do
		{
			rEncoderPool->reset();
			auto encoder = rEncoderPool->allocateEncoder(EncoderFlag::RenderEncoder);
			auto renderEncoder = encoder->asRenderEncoder();

			encoder->begin();
			colourRT0->transitionToDMADest(encoder);
			renderEncoder->clearTexture(colourRT0, {1.0f, 0.0f, 0.0f, 1.0f});

			colourRT0->transitionToRenderTarget(encoder);
			renderEncoder->beginRenderPass(renderPass, renderTarget);
			renderEncoder->bind(renderPipeline);
			renderEncoder->bindVertexBuffer(vBuffer);
			renderEncoder->pushConstants(renderPipeline, PushConstantRange{64, 16, ShaderType::Fragment,}, &white);
			renderEncoder->draw(3);
			renderEncoder->bindVertexBuffer(vBuffer2);
			renderEncoder->bindIndexBuffer(iBuffer);
			renderEncoder->pushConstants(renderPipeline, PushConstantRange{64, 16, ShaderType::Fragment,}, &red);
			renderEncoder->drawIndexed(3);
			renderEncoder->endRenderPass();
			colourRT0->transitionToDMASrc(encoder);

			encoder->end();

			renderQueue->enqueue(encoder);
			renderQueue->submit();
			renderQueue->stallTillIdle();
			device->houseKeepTick();
			display->present(colourRT0);
			if(Input::g_Keyboard)
			{
				using namespace Input;
				if(Input::g_Keyboard->keyDown(Key::KT_ESCAPE)) return true;
			}
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
	App app(shell_);

	if(!app.init()) return 10;

	bool okay = app.body();

	app.finish();
	if(!okay) return 10;
	else return 0;
}