#pragma once
#ifndef WYRD_RENDER_RESOURCEHANDLERS_H
#define WYRD_RENDER_RESOURCEHANDLERS_H

#include "core/core.h"
#include "resourcemanager/resourceid.h"

namespace ResourceManager {
class ResourceMan;

template<ResourceId>
struct ResourceHandle;
}

namespace Render {

struct GenericImage;
struct Texture;
struct RenderPass;
struct RenderTarget;
struct SPIRVShader;
struct RenderPipeline;
struct ComputePipeline;
struct BindingTable;
struct BindingTableMemoryMap;
struct Sampler;
struct ROPBlender;
struct Viewport;
struct VertexInput;

auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_) -> void;

using ResourceManager::operator ""_resource_id;

// render resource Ids
static constexpr ResourceManager::ResourceId GenericImageId = "GIMG"_resource_id;
static constexpr ResourceManager::ResourceId RenderPassId = "RPAS"_resource_id;
static constexpr ResourceManager::ResourceId RenderTargetId = "RTGT"_resource_id;
static constexpr ResourceManager::ResourceId TextureId = "TXTR"_resource_id;
static constexpr ResourceManager::ResourceId SPIRVShaderId = "SPRV"_resource_id;
static constexpr ResourceManager::ResourceId RenderPipelineId = "RPIP"_resource_id;
static constexpr ResourceManager::ResourceId ComputePipelineId = "CPIP"_resource_id;
static constexpr ResourceManager::ResourceId BindingTableMemoryMapId = "BNDM"_resource_id;
static constexpr ResourceManager::ResourceId BindingTableId = "BIND"_resource_id;
static constexpr ResourceManager::ResourceId SamplerId = "SAMP"_resource_id;
static constexpr ResourceManager::ResourceId ROPBlenderId = "ROPB"_resource_id;
static constexpr ResourceManager::ResourceId ViewportId = "VIEW"_resource_id;
static constexpr ResourceManager::ResourceId VertexInputId = "VINP"_resource_id;

using GenericImageHandle = ResourceManager::ResourceHandle<GenericImageId>;
using RenderPassHandle = ResourceManager::ResourceHandle<RenderPassId>;
using RenderTargetHandle = ResourceManager::ResourceHandle<RenderTargetId>;
using TextureHandle = ResourceManager::ResourceHandle<TextureId>;
using SPIRVShaderHandle = ResourceManager::ResourceHandle<SPIRVShaderId>;
using RenderPipelineHandle = ResourceManager::ResourceHandle<RenderPipelineId>;
using ComputePipelineHandle = ResourceManager::ResourceHandle<ComputePipelineId>;
using BindingTableMemoryMapHandle = ResourceManager::ResourceHandle<BindingTableMemoryMapId>;
using BindingTableHandle = ResourceManager::ResourceHandle<BindingTableId>;
using SamplerHandle = ResourceManager::ResourceHandle<SamplerId>;
using ROPBlenderHandle = ResourceManager::ResourceHandle<ROPBlenderId>;
using ViewportHandle = ResourceManager::ResourceHandle<ViewportId>;
using VertexInputHandle = ResourceManager::ResourceHandle<VertexInputId>;

using GenericImagePtr = std::shared_ptr<GenericImage>;
using RenderPassPtr = std::shared_ptr<RenderPass>;
using RenderTargetPtr = std::shared_ptr<RenderTarget>;
using TexturePtr = std::shared_ptr<Texture>;
using SPIRVShaderPtr = std::shared_ptr<SPIRVShader>;
using RenderPipelinePtr = std::shared_ptr<RenderPipeline>;
using ComputePipelinePtr = std::shared_ptr<ComputePipeline>;
using BindingTableMemoryMapPtr = std::shared_ptr<BindingTableMemoryMap>;
using BindingTablePtr = std::shared_ptr<BindingTable>;
using SamplerPtr = std::shared_ptr<Sampler>;
using ROPBlenderPtr = std::shared_ptr<ROPBlender>;
using ViewportPtr = std::shared_ptr<Viewport>;
using VertexInputPtr = std::shared_ptr<VertexInput>;

using GenericImageConstPtr = std::shared_ptr<GenericImage const>;
using RenderPassConstPtr = std::shared_ptr<RenderPass const>;
using RenderTargetConstPtr = std::shared_ptr<RenderTarget const>;
using TextureConstPtr = std::shared_ptr<Texture const>;
using SPIRVShaderConstPtr = std::shared_ptr<SPIRVShader const>;
using RenderPipelineConstPtr = std::shared_ptr<RenderPipeline const>;
using ComputePipelineConstPtr = std::shared_ptr<ComputePipeline const>;
using BindingTableMemoryMapConstPtr = std::shared_ptr<BindingTableMemoryMap const>;
using BindingTableConstPtr = std::shared_ptr<BindingTable const>;
using SamplerConstPtr = std::shared_ptr<Sampler const>;
using ROPBlenderConstPtr = std::shared_ptr<ROPBlender const>;
using ViewportConstPtr = std::shared_ptr<Viewport const>;
using VertexInputConstPtr = std::shared_ptr<VertexInput const>;

}


#endif //WYRD_RENDER_RESOURCEHANDLERS_H
