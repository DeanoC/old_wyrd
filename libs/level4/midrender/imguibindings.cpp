
#include "core/core.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "render/bindingtable.h"
#include "render/buffer.h"
#include "render/encoder.h"
#include "render/sampler.h"
#include "render/shader.h"
#include "render/pipeline.h"
#include "render/rasterisationstate.h"
#include "render/texture.h"
#include "render/vertexinput.h"
#include "render/viewport.h"
#include "midrender/stocks.h"
#include "resourcemanager/resourceman.h"
#include "midrender/imguibindings.h"

namespace MidRender {

namespace { // anon
// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
std::vector<uint32_t> glsl_shader_vert_spv{
		0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
		0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
		0x000a000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015,
		0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
		0x00000000, 0x00030005, 0x00000009, 0x00000000, 0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43,
		0x00000072, 0x00040006, 0x00000009, 0x00000001, 0x00005655, 0x00030005, 0x0000000b, 0x0074754f,
		0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015, 0x00565561, 0x00060005,
		0x00000019, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000019, 0x00000000,
		0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
		0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368, 0x6e617473, 0x00000074,
		0x00050006, 0x0000001e, 0x00000000, 0x61635375, 0x0000656c, 0x00060006, 0x0000001e, 0x00000001,
		0x61725475, 0x616c736e, 0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b,
		0x0000001e, 0x00000000, 0x00040047, 0x0000000f, 0x0000001e, 0x00000002, 0x00040047, 0x00000015,
		0x0000001e, 0x00000001, 0x00050048, 0x00000019, 0x00000000, 0x0000000b, 0x00000000, 0x00030047,
		0x00000019, 0x00000002, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e,
		0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001, 0x00000023, 0x00000008,
		0x00030047, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
		0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
		0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007, 0x00000008, 0x00040020,
		0x0000000a, 0x00000003, 0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x00040015,
		0x0000000c, 0x00000020, 0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020,
		0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040020,
		0x00000011, 0x00000003, 0x00000007, 0x0004002b, 0x0000000c, 0x00000013, 0x00000001, 0x00040020,
		0x00000014, 0x00000001, 0x00000008, 0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020,
		0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007, 0x00040020, 0x0000001a,
		0x00000003, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x0004003b, 0x00000014,
		0x0000001c, 0x00000001, 0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
		0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009, 0x00040020, 0x00000021,
		0x00000009, 0x00000008, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b, 0x00000006,
		0x00000029, 0x3f800000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
		0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000f, 0x00050041, 0x00000011, 0x00000012,
		0x0000000b, 0x0000000d, 0x0003003e, 0x00000012, 0x00000010, 0x0004003d, 0x00000008, 0x00000016,
		0x00000015, 0x00050041, 0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018,
		0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041, 0x00000021, 0x00000022,
		0x00000020, 0x0000000d, 0x0004003d, 0x00000008, 0x00000023, 0x00000022, 0x00050085, 0x00000008,
		0x00000024, 0x0000001d, 0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
		0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008, 0x00000027, 0x00000024,
		0x00000026, 0x00050051, 0x00000006, 0x0000002a, 0x00000027, 0x00000000, 0x00050051, 0x00000006,
		0x0000002b, 0x00000027, 0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b,
		0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b, 0x0000000d, 0x0003003e,
		0x0000002d, 0x0000002c, 0x000100fd, 0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
std::vector<uint32_t> glsl_shader_frag_spv{
		0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
		0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
		0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010,
		0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
		0x00000000, 0x00040005, 0x00000009, 0x6c6f4366, 0x0000726f, 0x00030005, 0x0000000b, 0x00000000,
		0x00050006, 0x0000000b, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x0000000b, 0x00000001,
		0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016, 0x78655473, 0x65727574,
		0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x0000001e,
		0x00000000, 0x00040047, 0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
		0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
		0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
		0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006,
		0x00000002, 0x0004001e, 0x0000000b, 0x00000007, 0x0000000a, 0x00040020, 0x0000000c, 0x00000001,
		0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000001, 0x00040015, 0x0000000e, 0x00000020,
		0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001,
		0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
		0x00000001, 0x00000000, 0x0003001b, 0x00000014, 0x00000013, 0x00040020, 0x00000015, 0x00000000,
		0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
		0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036, 0x00000002, 0x00000004,
		0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000010, 0x00000011, 0x0000000d,
		0x0000000f, 0x0004003d, 0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017,
		0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018, 0x0004003d, 0x0000000a,
		0x0000001b, 0x0000001a, 0x00050057, 0x00000007, 0x0000001c, 0x00000017, 0x0000001b, 0x00050085,
		0x00000007, 0x0000001d, 0x00000012, 0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd,
		0x00010038
};

} // end anon namespace


auto ImguiBindings::init(std::shared_ptr<ResourceManager::ResourceMan>& rm_) -> void
{
	using namespace Render;
	using namespace ResourceManager;
	using namespace Core::bitmask;

	rm = rm_;

	vertexShaderHandle = SPIRVShader::Create(
			rm_,
			ResourceNameView("mem$ImguiVertexShader"),
			{},
			ShaderSourceLanguage::GLSL,
			ShaderType::Vertex,
			0,
			glsl_shader_vert_spv);

	fragmentShaderHandle = SPIRVShader::Create(
			rm_,
			ResourceNameView("mem$ImguiFragmentShader"),
			{},
			ShaderSourceLanguage::GLSL,
			ShaderType::Fragment,
			0,
			glsl_shader_frag_spv);

	fontSamplerHandle = Sampler::Create(
			rm_,
			ResourceNameView("mem$ImguiFontSampler"),
			Filter::Linear,
			Filter::Linear,
			Filter::Linear,
			SamplerAddressMode::Repeat,
			SamplerAddressMode::Repeat);

	memoryMapHandle = BindingTableMemoryMap::Create(
			rm_,
			ResourceNameView("mem$ImguiBindingTableMemoryMap"),
			{
				{BindingTableType::CombinedTextureSampler, 1, ShaderType::Fragment }
			}
			);

	bindingTableHandle = BindingTable::Create(
			rm_,
			ResourceNameView("mem$ImguiBindingTable"),
			{
				{ memoryMapHandle }
			}
			);

	vertexFormatHandle = VertexInput::Create(
			rm_,
			ResourceNameView("mem$ImguiVertexFormat"),
			{
				{VertexInputLocation(Position), VertexInputType::Float2},
				{VertexInputLocation(1), VertexInputType::Float2},
				{VertexInputLocation(2), VertexInputType::Byte4}
			});

	pipelineHandle = RenderPipeline::Create(
			rm_,
			ResourceNameView("mem$ImguiRenderPipeline"),
			Topology::Triangles,
			RenderPipelineFlags::None,
			DynamicPipelineState::Viewport | DynamicPipelineState::Scissor,
			{ memoryMapHandle },
			{
				{0, sizeof(float)*4, ShaderType::Vertex}
			},
			{
				vertexShaderHandle,
				fragmentShaderHandle
			},
			rm_->openByName<RasterisationStateId>(Stock::defaultRasterState),
			rm_->openByName<RenderPassId>(Stock::simpleForwardRendererRenderPass),
			rm_->openByName<ROPBlenderId>(Stock::singleOverROPBlender),
			vertexFormatHandle,
			rm_->openByName<ViewportId>(Stock::simpleForwardRendererViewport)
	);

	IMGUI_CHECKVERSION();
	context = ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	size_t const upload_size = width*height*4*sizeof(char);
	auto image = GenericImage::Create(
			rm_,
			ResourceNameView("mem$ImguiFontImage"),
			width,
			height,
			1,
			1,
			GenericTextureFormat::R8G8B8A8_UNORM,
			pixels
	);

	fontTextureHandle = Texture::Create(
			rm_,
			ResourceNameView("mem$ImguiFontTexture"),
			Texture::FromUsage(Usage::DMADst | Usage::ShaderRead),
			width,
			height,
			1,
			1,
			1,
			1,
			GenericTextureFormat::R8G8B8A8_UNORM,
			image
	);
	io.Fonts->TexID = fontTextureHandle;

	// setup back-end capability flags
	//	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

	// TODO
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			IM_ASSERT(platform_io.Platform_CreateVkSurface != NULL && "Platform needs to setup the CreateVkSurface handler.");

//		platform_io.Renderer_CreateWindow = ImGui_ImplVulkan_CreateWindow;
//		platform_io.Renderer_DestroyWindow = ImGui_ImplVulkan_DestroyWindow;
//		platform_io.Renderer_SetWindowSize = ImGui_ImplVulkan_SetWindowSize;
//		platform_io.Renderer_RenderWindow = ImGui_ImplVulkan_RenderWindow;
//		platform_io.Renderer_SwapBuffers = ImGui_ImplVulkan_SwapBuffers;
	}

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
	using namespace Input;
	io.KeyMap[ImGuiKey_Tab] = (uint16_t) Key::KT_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = (uint16_t) Key::KT_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = (uint16_t) Key::KT_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = (uint16_t) Key::KT_UP;
	io.KeyMap[ImGuiKey_DownArrow] = (uint16_t) Key::KT_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = (uint16_t) Key::KT_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = (uint16_t) Key::KT_NEXT;
	io.KeyMap[ImGuiKey_Home] = (uint16_t) Key::KT_HOME;
	io.KeyMap[ImGuiKey_End] = (uint16_t) Key::KT_END;
	io.KeyMap[ImGuiKey_Insert] = (uint16_t) Key::KT_INSERT;
	io.KeyMap[ImGuiKey_Delete] = (uint16_t) Key::KT_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = (uint16_t) Key::KT_BACK;
	io.KeyMap[ImGuiKey_Space] = (uint16_t) Key::KT_SPACE;
	io.KeyMap[ImGuiKey_Enter] = (uint16_t) Key::KT_RETURN;
	io.KeyMap[ImGuiKey_Escape] = (uint16_t) Key::KT_ESCAPE;
	io.KeyMap[ImGuiKey_A] = (uint16_t) Key::KT_A;
	io.KeyMap[ImGuiKey_C] = (uint16_t) Key::KT_C;
	io.KeyMap[ImGuiKey_V] = (uint16_t) Key::KT_V;
	io.KeyMap[ImGuiKey_X] = (uint16_t) Key::KT_X;
	io.KeyMap[ImGuiKey_Y] = (uint16_t) Key::KT_Y;
	io.KeyMap[ImGuiKey_Z] = (uint16_t) Key::KT_Z;

	// TODO cross platform timers
#if PLATFORM == WINDOWS
	::QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond);
	::QueryPerformanceCounter((LARGE_INTEGER *)&time);
#endif
}

auto ImguiBindings::destroy() -> void
{
}
auto ImguiBindings::newFrame(uint32_t width_, uint32_t height_) -> void
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = width_;
	io.DisplaySize.y = height_;

	// TODO cross platform timers
#if PLATFORM == WINDOWS
	// Setup time step
	uint64_t current_time;
	::QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
	io.DeltaTime = (float)(current_time - time) / ticksPerSecond;
	time = current_time;
#endif

	using namespace Input;
	if(g_Keyboard != nullptr)
	{
		std::memcpy(io.KeysDown, g_Keyboard->getKeyDownBitmap(), sizeof(bool) * Keyboard::MaxKeyCount);
		io.KeyCtrl = (g_Keyboard->keyDown(Key::KT_LCONTROL) || g_Keyboard->keyDown(Key::KT_RCONTROL));
		io.KeyAlt = (g_Keyboard->keyDown(Key::KT_LMENU) || g_Keyboard->keyDown(Key::KT_RMENU));
		io.KeyShift = (g_Keyboard->keyDown(Key::KT_LSHIFT) || g_Keyboard->keyDown(Key::KT_RSHIFT));
		io.KeySuper = (g_Keyboard->keyDown(Key::KT_LWIN) || g_Keyboard->keyDown(Key::KT_RWIN));
		// TODO AddInputCharacter
	}
	if(g_Mouse != nullptr)
	{
		io.MousePos[0] = g_Mouse->getAbsMouseX();
		io.MousePos[1] = g_Mouse->getAbsMouseY();
		io.MouseDown[0] = g_Mouse->buttonDown(MouseButton::Left);
		io.MouseDown[1] = g_Mouse->buttonDown(MouseButton::Middle);
		io.MouseDown[2] = g_Mouse->buttonDown(MouseButton::Right);
	}

	ImGui::NewFrame();
}

auto ImguiBindings::render(std::shared_ptr<Render::Encoder>& encoder_) -> void
{
	using namespace Render;
	using namespace Core::bitmask;
	using namespace ResourceManager;

	ImGui::EndFrame();
	ImGui::Render();
	auto drawData = ImGui::GetDrawData();

	assert(drawData);
	if (drawData->TotalVtxCount == 0)
		return;

	// Create the Vertex and Index buffers:
	size_t const vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
	size_t const indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

	if (allocatedVertexBufferSize < vertexSize)
	{
		// TODO add the ability to hint resource manager to dump the old resource
		std::string name("mem$ImguiVertexBuffer_gen");
		name += std::to_string(vertexBufferAllocGeneration++);
		auto handle = Buffer::Create(
				rm,
				ResourceNameView(name),
				BufferFlags::NoInit | BufferFlags::CPUDynamic |
				Buffer::FromUsage(Usage::DMADst | Usage::VertexRead),
				vertexSize);
		vertexBuffer = handle.acquire<Buffer>();
		allocatedVertexBufferSize = vertexSize;
	}

	if (allocatedIndexBufferSize < indexSize)
	{
		std::string name("mem$ImguiIndexBuffer_gen");
		name += std::to_string(indexBufferAllocGeneration++);
		auto handle = Buffer::Create(
				rm,
				ResourceNameView(name),
				BufferFlags::NoInit | BufferFlags::CPUDynamic |
				Buffer::FromUsage(Usage::DMADst | Usage::IndexRead),
				indexSize);
		indexBuffer = handle.acquire<Buffer>();
		allocatedIndexBufferSize = indexSize;
	}

	// upload vertex and index data
	ImDrawVert* vtx_dst = (ImDrawVert*)vertexBuffer->map();
	ImDrawIdx* idx_dst = (ImDrawIdx*)indexBuffer->map();
	assert(vtx_dst);
	assert(idx_dst);
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	indexBuffer->unmap();
	vertexBuffer->unmap();


	// update bindingTable for texture and sampler
	auto bindingTable = bindingTableHandle.acquire<BindingTable>();
	bindingTable->update(0, 0, fontTextureHandle, fontSamplerHandle );

	// bind stuff
	auto renderEncoder = encoder_->asRenderEncoder();
	auto renderPipeline = pipelineHandle.acquire<RenderPipeline>();
	renderEncoder->bind(renderPipeline, bindingTable);
	renderEncoder->bindVertexBuffer(vertexBuffer);
	renderEncoder->bindIndexBuffer(indexBuffer);

	// set push constants used for scaling
	std::array<float,2> scale {
			2.0f / drawData->DisplaySize.x,
			2.0f / drawData->DisplaySize.y
	};
	std::array<float, 2> translate {
			-1.0f - drawData->DisplayPos.x * scale[0],
			-1.0f - drawData->DisplayPos.y * scale[1]
	};

	ViewportDef viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = drawData->DisplaySize.x;
	viewport.height = drawData->DisplaySize.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	renderEncoder->setDynamicViewport(0, viewport);

	renderEncoder->pushConstants(
			renderPipeline,
			PushConstantRange{0, sizeof(float)*2, ShaderType::Vertex,},
			scale.data());
	renderEncoder->pushConstants(
			renderPipeline,
			PushConstantRange{sizeof(float)*2, sizeof(float)*2, ShaderType::Vertex,},
			translate.data());

	// draw lists setting scissor as we go
	// Render the command lists:
	int vtx_offset = 0;
	int idx_offset = 0;
	ImVec2 display_pos = drawData->DisplayPos;
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		for(int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if(pcmd->UserCallback)
			{
//				pcmd->UserCallback(cmd_list, pcmd);
			} else
			{
				// Apply scissor/clipping rectangle
				// FIXME: We could clamp width/height based on clamped min/max values.
				Render::Scissor scissor;
				scissor.offset[0] =
						(int32_t) (pcmd->ClipRect.x - display_pos.x) > 0 ? (int32_t) (pcmd->ClipRect.x - display_pos.x)
																		 : 0;
				scissor.offset[1] =
						(int32_t) (pcmd->ClipRect.y - display_pos.y) > 0 ? (int32_t) (pcmd->ClipRect.y - display_pos.y)
																		 : 0;
				scissor.extent[0] = (uint32_t) (pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissor.extent[1] = (uint32_t) (pcmd->ClipRect.w - pcmd->ClipRect.y + 1); // FIXME: Why +1 here?
				renderEncoder->setDynamicScissor(0, scissor);
				// Draw
				renderEncoder->drawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}
}


} // end MidRender namespace