#pragma
#ifndef WYRD_MIDRENDER_IMGUIBINDINGS_H
#define WYRD_MIDRENDER_IMGUIBINDINGS_H

#include "core/core.h"
#include "render/resources.h"

#include "imgui/imgui.h"

namespace ResourceManager { class ResourceMan; }

namespace MidRender {
struct Encoder;

struct ImguiBindings
{
	auto init(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void;
	auto destroy() -> void;

	auto newFrame(uint32_t width_, uint32_t height_) -> void;
	auto render(std::shared_ptr<Render::Encoder>& encoder_) -> void;

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

	uint64_t time;
	uint64_t ticksPerSecond;

	ImGuiContext* context;
};

}

#endif //WYRD_MIDRENDER_IMGUIBINDINGS_H
