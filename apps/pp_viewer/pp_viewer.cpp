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
#include "midrender/imguibindings.h"
#include "midrender/simpleforwardglobals.h"
#include "midrender/simpleeye.h"
#include "midrender/meshmodrenderer.h"
#include "meshmod/scenenode.h"
#include "meshmod/mesh.h"
#include "meshops/platonicsolids.h"
#include "meshops/shapes.h"

#include "timing/tickerclock.h"
#include "replay/replay.h"
#include "replay/gui.h"

#include "server.h"
#include "fakeclient.h"

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
		shellConfig.appName = "pp_viewer"s;
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

		std::string windowName = "Phoeonix Pointer Remote Viewer"s;
		Shell::PresentableWindowConfig windowConfig;
		windowConfig.width = 1280;
		windowConfig.height = 720;
		windowConfig.windowName = windowName;
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

		weakDisplay = device->getDisplay();

		createResources();

		tickerClock = std::make_unique<Timing::TickerClock>();
		replay = std::make_shared<Replay::Replay>();
		replayGui = std::make_unique<Replay::Gui>(replay);
		server = std::make_unique<Server>(replay);
		client = std::make_unique<FakeClient>();

		return okay;
	}

	auto finish() -> void
	{
		client.reset();
		server.reset();
		replayGui.reset();
		replay.reset();
		tickerClock.reset();

		imguiBindings->destroy();
		imguiBindings.reset();
		meshModRenderer->destroy();
		meshModRenderer.reset();

		resourceManager->flushCache();
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
		meshModRenderer.reset(new MidRender::MeshModRenderer());
		meshModRenderer->init(rm);

		auto rootScene = std::make_shared<MeshMod::SceneNode>();
		rootScene->addObject(MeshOps::Shapes::createDiamond());
		solidSceneIndex = meshModRenderer->addScene(rootScene);
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

		auto simpleEye = new SimpleEye();
		simpleEye->setProjection(60.0f, 1280.0f / 720.0f, 0.01f);
		Math::mat4x4 view = Math::lookAt(Math::vec3(0, 0, 4),
										 Math::vec3(0, 0, 0),
										 Math::vec3(0, 1, 0));
		simpleEye->setView(view);
		tickerClock->update();

		float yrot = 0.0f;
		using namespace Core::bitmask;
		do
		{
			auto deltaT = tickerClock->update();
			replay->update(deltaT);

			rEncoderPool->reset();

			Math::mat4x4 rootMatrix = Math::rotate(Math::identity<Math::mat4x4>(), yrot, Math::vec3(0, 1, 0));
			yrot += Math::degreesToRadians(90.0f) * float(deltaT);
			while(yrot > Math::two_pi<float>())
			{
				yrot -= Math::two_pi<float>();
			}

			SimpleForwardGlobals* globals = (SimpleForwardGlobals*) globalBuffer->map();
			std::memcpy(globals->viewMatrix, &simpleEye->getView(), sizeof(float) * 16);
			std::memcpy(globals->projectionMatrix, &simpleEye->getProjection(), sizeof(float) * 16);
			auto viewProj = simpleEye->getProjection() * simpleEye->getView();
			std::memcpy(globals->viewProjectionMatrix, &viewProj, sizeof(float) * 16);
			globalBuffer->unmap();

			imguiBindings->newFrame(display->getWidth(), display->getHeight());

			replayGui->render();

			bool show_demo_window = true;
			bool show_app_about = true;

			if(show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);
			auto encoder = rEncoderPool->allocateEncoder(EncoderFlag::RenderEncoder);
			auto renderEncoder = encoder->asRenderEncoder();

			encoder->begin();
			colourRT0->transitionToRenderTarget(encoder);
			renderEncoder->beginRenderPass(renderPass, renderTarget);

			meshModRenderer->render(rootMatrix, solidSceneIndex, encoder);
			imguiBindings->render(encoder);

			renderEncoder->endRenderPass();
			colourRT0->transitionToDMASrc(encoder);

			encoder->end();

			renderQueue->enqueue(encoder);
			renderQueue->submit();
			renderQueue->stallTillIdle();

			encoder.reset();

			client->update();
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

	std::unique_ptr<MidRender::ImguiBindings> imguiBindings;
	std::unique_ptr<MidRender::MeshModRenderer> meshModRenderer;
	MidRender::MeshModRenderer::SceneIndex solidSceneIndex;
	std::unique_ptr<Server> server;
	std::unique_ptr<FakeClient> client;

	std::shared_ptr<Replay::Replay> replay;
	std::unique_ptr<Replay::Gui> replayGui;
	std::unique_ptr<Timing::TickerClock> tickerClock;
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