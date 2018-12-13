#include "core/core.h"
#include "gfxtest.h"
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
#include "midrender/imguibindings.h"
#include "midrender/simpleforwardglobals.h"
#include "midrender/simpleeye.h"
#include "midrender/meshmodrenderer.h"
#include "meshops/platonicsolids.h"
#include "meshmod/mesh.h"
#include "meshmod/scenenode.h"
#include "input/keyboard.h"
#include "timing/tickerclock.h"
#include "meshops/shapes.h"
#include "input/provider.h"
#include "input/vpadlistener.h"
#include "meshops/gltf.h"
#include "appcommon/simplepadcamera.h"
#include "appcommon/arcballcamera.h"


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
		shellConfig.appName = "Gfx Test"s;
		shellConfig.wantConsoleOutput = false;
		shellConfig.gpuComputeRequired = true;
		shellConfig.gpuRenderRequired = true;
		bool okay = shell.init(shellConfig);
		if(!okay)
		{
			LOG_S(WARNING) << "Shell init config failed";
			return false;
		}

		resourceManager = ResourceManager::ResourceMan::Create();
		auto memstorage = std::make_shared<ResourceManager::MemStorage>();
		resourceManager->registerStorageHandler(memstorage);

		RegisterResourceHandlers(*resourceManager);

		// use a presentable and descrete if possible else integrated presentable
		Stable* gpuStable = shell.getGpuStable();
		if(gpuStable == nullptr)
		{
			LOG_S(WARNING) << "Shell has no gpu stable";
			return false;
		}

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
		windowConfig.windowName = "Gfx Test"s;
		windowConfig.fullscreen = false;
		windowConfig.directInput = true;
		window = shell.createPresentableWindow(windowConfig);
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

		weakDisplay = device->getDisplay();

		tickerClock = std::make_unique<Timing::TickerClock>();
		createResources();

		underTest = CreateBasicMeshTest();
//		underTest = CreateDuckGltfTest();
//		underTest = CreateSponzaGltfTest();
//		underTest = CreateTacmapGltfTest();
		underTest->init(resourceManager);

		return okay;
	}

	auto finish() -> void
	{
		underTest->finish();

		imguiBindings->destroy();
		imguiBindings.reset();

		resourceManager->flushCache();

		tickerClock.reset();
		weakDisplay.reset();
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

		auto display = weakDisplay.lock();
		// alias resourceManager name (why can't I use 'using' C++ standard body!)
		auto& rm = resourceManager;
		MidRender::Stocks::InitBasics(rm);
		MidRender::Stocks::InitSimpleForwardRenderer(rm,
													 display->getWidth(),
													 display->getHeight());

		imguiBindings.reset(new MidRender::ImguiBindings());
		imguiBindings->init(rm);

	}

	auto body() -> bool
	{
		using namespace Render;
		using namespace MidRender;;
		using namespace std::string_view_literals;

		auto display = weakDisplay.lock();

		// acquire the resources by name
		auto& rm = resourceManager;
		auto colourRT0 = rm->acquireByName<Texture>(Stock::simpleForwardColourRT);
		auto renderPass = rm->acquireByName<RenderPass>(Stock::simpleForwardRenderPass);
		auto renderTarget = rm->acquireByName<RenderTarget>(Stock::simpleForwardRenderTarget);

		auto globalBuffer = rm->acquireByName<Buffer>(Stock::simpleForwardGlobalBuffer);

		auto renderQueue = device->getGeneralQueue();
		auto rEncoderPool = device->makeEncoderPool(true, CommandQueueFlavour::Render);

		simplePadCamera = std::make_shared<SimplePadCamera>();
		arcBallCamera = std::make_unique<ArcBallCamera>();

		inputProvider = shell.getInputProvider(window);
		inputProvider->setVirtualPadListener(0, simplePadCamera);
		tickerClock->update();

		using namespace Core::bitmask;
		do
		{
			auto deltaT = tickerClock->update();
			inputProvider->update(deltaT);
			simplePadCamera->update(deltaT);
			arcBallCamera->update(deltaT);

			rEncoderPool->reset();

			auto const simpleEye = &arcBallCamera->simpleEye;
			SimpleForwardGlobals* globals = (SimpleForwardGlobals*) globalBuffer->map();
			std::memcpy(globals->viewMatrix, &simpleEye->getView(), sizeof(float) * 16);
			std::memcpy(globals->projectionMatrix, &simpleEye->getProjection(), sizeof(float) * 16);
			auto viewProj = simpleEye->getProjection() * simpleEye->getView();
			std::memcpy(globals->viewProjectionMatrix, &viewProj, sizeof(float) * 16);
			globalBuffer->unmap();

			imguiBindings->newFrame(display->getWidth(), display->getHeight());

			auto encoder = rEncoderPool->allocateEncoder(EncoderFlag::RenderEncoder);
			auto renderEncoder = encoder->asRenderEncoder();

			encoder->begin();
			colourRT0->transitionToRenderTarget(encoder);
			renderEncoder->beginRenderPass(renderPass, renderTarget);

			underTest->tick(deltaT, encoder);
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Camera Controls"))
				{
					if (ImGui::MenuItem("WASD FPS")) {}
					if (ImGui::MenuItem("ArcBall")) {}
//					ImGui::Separator();
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}

			imguiBindings->render(encoder);
			renderEncoder->endRenderPass();
			colourRT0->transitionToDMASrc(encoder);
			encoder->end();

			renderQueue->enqueue(encoder);
			renderQueue->submit();
			renderQueue->stallTillIdle();

			encoder.reset();

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
	Render::Display::WeakPtr weakDisplay;
	Shell::PresentableWindow* window;
	std::unique_ptr<Timing::TickerClock> tickerClock;
	std::unique_ptr<Input::Provider> inputProvider;
	std::shared_ptr<SimplePadCamera> simplePadCamera;
	std::unique_ptr<ArcBallCamera> arcBallCamera;

	std::unique_ptr<MidRender::ImguiBindings> imguiBindings;
	std::unique_ptr<GfxTest> underTest;
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