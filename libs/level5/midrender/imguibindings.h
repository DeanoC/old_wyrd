#pragma
#ifndef WYRD_MIDRENDER_IMGUIBINDINGS_H
#define WYRD_MIDRENDER_IMGUIBINDINGS_H

#include "core/core.h"
#include "render/resources.h"
#include "IconFontCppHeaders/IconsFontAwesome5.h"
#include "imgui/imgui.h"

namespace ResourceManager { class ResourceMan; }
namespace Timing { class TickerClock; }

namespace MidRender {
struct Encoder;

struct ImguiBindings
{
	auto init(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void;
	auto destroy() -> void;

	auto newFrame(uint32_t width_, uint32_t height_) -> void;
	auto render(std::shared_ptr<Render::Encoder>& encoder_) -> void;

	auto wantCapturedKeyboard() const -> bool { return ImGui::GetIO().WantCaptureKeyboard; }
	auto wantCapturedMouse() const -> bool { return ImGui::GetIO().WantCaptureMouse; }

	std::shared_ptr<ResourceManager::ResourceMan> rm;

	Render::SPIRVShaderHandle vertexShaderHandle;
	Render::SPIRVShaderHandle fragmentShaderHandle;
	Render::BindingTableMemoryMapHandle memoryMapHandle;
	Render::BindingTableHandle bindingTableHandle;
	Render::SamplerHandle fontSamplerHandle;
	Render::VertexInputHandle vertexFormatHandle;
	Render::TextureHandle fontTextureHandle;
	Render::RenderPipelineHandle pipelineHandle;

	uint64_t allocatedVertexBufferSize = 0;
	uint64_t allocatedIndexBufferSize = 0;
	Render::BufferConstPtr vertexBuffer;
	Render::BufferConstPtr indexBuffer;

	std::unique_ptr<Timing::TickerClock> tickerClock;

	ImGuiContext* context;
};

}

#endif //WYRD_MIDRENDER_IMGUIBINDINGS_H
