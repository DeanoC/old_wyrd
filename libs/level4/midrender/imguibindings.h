#pragma
#ifndef WYRD_MIDRENDER_IMGUIBINDINGS_H
#define WYRD_MIDRENDER_IMGUIBINDINGS_H

#include "core/core.h"
#include "render/resources.h"

#define ImTextureID Render::TextureHandle
#include "imgui/imgui.h"

namespace ResourceManager { class ResourceMan; }

namespace MidRender {

struct ImguiBindings
{
	auto init(std::shared_ptr<ResourceManager::ResourceMan>& rm_) -> bool;

	Render::SPIRVShaderHandle vertexShaderHandle;
	Render::SPIRVShaderHandle fragmentShaderHandle;
	Render::BindingTableMemoryMapHandle memoryMapHandle;
	Render::BindingTableHandle bindingTableHandle;
	Render::SamplerHandle fontSamplerHandle;
	Render::VertexInputHandle vertexFormatHandle;
	Render::TextureHandle fontTextureHandle;

	Render::RenderPipelineHandle pipelineHandle;

};

}

#endif //WYRD_MIDRENDER_IMGUIBINDINGS_H
