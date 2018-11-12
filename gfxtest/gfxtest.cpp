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

static auto defaultRenderPassName = ResourceManager::ResourceNameView("mem$defaultRenderPass");
static auto defaultRenderTargetName = ResourceManager::ResourceNameView("mem$defaultRenderTarget");
static auto defaultBindingTableMemoryMapName = ResourceManager::ResourceNameView("mem$defaultBindingTableMemoryMap");
static auto defaultBindingTableName = ResourceManager::ResourceNameView("mem$defaultBindingTable");
static auto defaultViewportName = ResourceManager::ResourceNameView("mem$defaultViewport");
static auto defaultVertexInputName = ResourceManager::ResourceNameView("mem$defaultVertexInput");
static auto defaultROPBlenderName = ResourceManager::ResourceNameView("mem$defaultROPBlender");
static auto defaultRasterStateName = ResourceManager::ResourceNameView("mem$defaultRasterState");
static auto defaultRenderPipelineName = ResourceManager::ResourceNameView("mem$defaultRenderPipeline");

static auto blankTex4x4Name = ResourceManager::ResourceNameView("mem$blankTex4x4");
static auto basicTriVertexBufferName = ResourceManager::ResourceNameView("mem$basicTriVertexBuffer");
static auto basicTriVertexBuffer2Name = ResourceManager::ResourceNameView("mem$basicTriVertexBuffer2");
static auto basicTriIndexBufferName = ResourceManager::ResourceNameView("mem$basicTriIndexBuffer");
static auto colourRT0Name = ResourceManager::ResourceNameView("mem$colourRT0");
static auto passthroughVertexShaderName = ResourceManager::ResourceNameView("mem$passthroughVertexShader");
static auto redOutFragmentShaderName = ResourceManager::ResourceNameView("mem$redOutFragmentShader");
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

		// alias resourceManager name (why can't I use 'using' C++ standard body!)
		auto& rm = resourceManager;
		Texture::Create(rm,
						blankTex4x4Name,
						TextureFlags::InitZero |
						Texture::FromUsage(Usage::ShaderRead | Usage::DMADst),
						4, 4, 1, 1,
						1, 1,
						GenericTextureFormat::R8G8B8A8_UNORM);

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
		Texture::Create(rm,
						colourRT0Name,
						TextureFlags::NoInit |
						Texture::FromUsage(Usage::RopRead | Usage::RopWrite | Usage::DMASrc | Usage::DMADst),
						display->getWidth(), display->getHeight(), 1, 1,
						1, 1,
						GenericTextureFormat::R8G8B8A8_UNORM);

		RenderPass::Create(rm,
						   defaultRenderPassName,
						   {{
									LoadOp::Clear,
									StoreOp::Store,
									GenericTextureFormat::R8G8B8A8_UNORM,
							}},
						   {0, 0, 0xFF, 0xFF});

		RenderTarget::Create(rm,
							 defaultRenderTargetName,
							 rm->openByName<RenderPassId>(defaultRenderPassName),
							 {rm->openByName<TextureId>(colourRT0Name)},
							 {0, 0},
							 {display->getWidth(), display->getHeight()}
		);

		auto const glslRedShaderSourceName = ResourceNameView("mem$glslRedOutFragmentShaderSource"sv);
		auto const hlslRedShaderSourceName = ResourceNameView("mem$hlslRedOutFragmentShaderSource"sv);
		auto const glslPassthroughSourceName = ResourceNameView("mem$glslPassthroughShaderSource"sv);
		auto const hlslPassthroughSourceName = ResourceNameView("mem$hlslPassthroughShaderSource"sv);

		auto const hlslPushColourShaderSourceName = ResourceNameView("mem$hlslPushColourOutFragmentShaderSource"sv);
		auto const glslPushColourShaderSourceName = ResourceNameView("mem$glslPushColourOutFragmentShaderSource"sv);

		TextResource::Create(rm,
							 glslPassthroughSourceName,
							 "#extension GL_ARB_separate_shader_objects : enable\n"
							 "#extension GL_ARB_shading_language_420pack : enable\n"
							 "layout (location = 0) in vec4 pos;\n"
							 "void main() {\n"
							 "   gl_Position = pos;\n"
							 "}\n");
		TextResource::Create(rm,
							 hlslPassthroughSourceName,
							 "float4 main([[vk::location(0)]] float4 pos) : SV_Position\n"
							 "{\n"
							 "		return pos;\n"
							 "}\n");
		TextResource::Create(rm,
							 glslRedShaderSourceName,
							 "#extension GL_ARB_separate_shader_objects : enable\n"
							 "#extension GL_ARB_shading_language_420pack : enable\n"
							 "layout (location = 0) out vec4 outColor;\n"
							 "void main() {\n"
							 "   outColor = vec4(1.0, 0.0,0.0, 1.0);\n"
							 "}\n");

		TextResource::Create(rm,
							 hlslRedShaderSourceName,
							 "[[vk::location(0)]] float4 main()\n"
							 "{\n"
							 "		return float4(1.0f, 0.0f, 0.0f, 1.0f);\n"
							 "}\n");

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
							 passthroughVertexShaderName,
							 {rm->openByName<TextResourceId>(glslPassthroughSourceName)},
							 ShaderSourceLanguage::GLSL,
							 ShaderType::Vertex,
							 0);

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

		Viewport::Create(rm,
						 defaultViewportName,
						 {{
								  0, 0,
								  (float) display->getWidth(), (float) display->getHeight(),
								  0.0f, 1.0f,
								  {0, 0},
								  {display->getWidth(), display->getHeight()}
						  }}
		);
		VertexInput::Create(rm,
							defaultVertexInputName,
							{
									{0, 0, VertexInputElement::Position, VertexInputType::Float3}
							});
		ROPBlender::Create(rm,
						   defaultROPBlenderName,
						   {
								   {
										   ROPTargetBlender::EnableFlag,
										   ROPBlendOps::Add,
										   ROPBlendOps::Add,
										   ColourComponents::All,
										   ROPBlendFactor::SrcColour,
										   ROPBlendFactor::Zero,
										   ROPBlendFactor::SrcAlpha,
										   ROPBlendFactor::Zero
								   }
						   },
						   {0.0f, 0.0f, 0.0f, 0.0f}
		);

		RasterisationState::Create(rm, defaultRasterStateName);

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
									   rm->openByName<SPIRVShaderId>(passthroughVertexShaderName),
									   rm->openByName<SPIRVShaderId>(pushColourFragmentShaderName),
							   },
							   rm->openByName<RasterisationStateId>(defaultRasterStateName),
							   rm->openByName<RenderPassId>(defaultRenderPassName),
							   rm->openByName<ROPBlenderId>(defaultROPBlenderName),
							   rm->openByName<ViewportId>(defaultViewportName),
							   rm->openByName<VertexInputId>(defaultVertexInputName)
		);
	}

	auto body() -> bool
	{
		using namespace Render;
		using namespace std::string_view_literals;

		// acquire the resources by name
		auto& rm = resourceManager;
		auto colourRT0 = rm->acquireByName<Texture>(colourRT0Name);
		auto defaultRenderPass = rm->acquireByName<RenderPass>(defaultRenderPassName);
		auto defaultRenderTarget = rm->acquireByName<RenderTarget>(defaultRenderTargetName);
		auto renderPipeline = rm->acquireByName<RenderPipeline>(defaultRenderPipelineName);

		auto blankTex = rm->acquireByName<Texture>(blankTex4x4Name);
		auto vBuffer = rm->acquireByName<Buffer>(basicTriVertexBufferName);
		auto vBuffer2 = rm->acquireByName<Buffer>(basicTriVertexBuffer2Name);
		auto iBuffer = rm->acquireByName<Buffer>(basicTriIndexBufferName);

		auto renderQueue = device->getGeneralQueue();
		auto rEncoderPool = device->makeEncoderPool(true, CommandQueueFlavour::Render);

		std::array<float, 4> const black{0.0f, 0.0f, 0.0f, 1.0f};
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
			renderEncoder->beginRenderPass(defaultRenderPass, defaultRenderTarget);
			renderEncoder->bind(renderPipeline);
			renderEncoder->bindVertexBuffer(vBuffer);
			renderEncoder->pushConstants(renderPipeline, PushConstantRange{64, 16, ShaderType::Fragment,}, &white);
			renderEncoder->draw(3);
			renderEncoder->bindVertexBuffer(vBuffer2);
			renderEncoder->bindIndexBuffer(iBuffer);
			renderEncoder->pushConstants(renderPipeline, PushConstantRange{64, 16, ShaderType::Fragment,}, &black);
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