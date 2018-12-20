#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/resourceman.h"
#include "binny/writehelper.h"
#include "render/pipeline.h"

namespace Render {
auto RenderPipeline::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

		auto renderPipeline = std::static_pointer_cast<RenderPipeline>(ptr_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

		for(auto i = 0u; i < renderPipeline->numBindingTableMemoryMaps; ++i)
		{
			resolverFunc(renderPipeline->getBindingTableMemoryMapHandles()[i].base);
		}

		for(auto i = 0u; i < renderPipeline->numShaders; ++i)
		{
			resolverFunc(renderPipeline->getSPIRVShaderHandles()[i].base);
		}

		return true;
	};

	auto destroy = [](int, void*)
	{
		return true;
	};
	auto changed = [](std::shared_ptr<ResourceBase const> ptr_)
	{
		return false;
	};
	auto save = [](std::shared_ptr<ResourceBase const> ptr_, ResourceManager::ISaver& writer_)
	{
		auto renderPipeline = std::static_pointer_cast<RenderPipeline const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto ComputePipeline::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto computePipeline = std::static_pointer_cast<ComputePipeline>(ptr_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

		resolverFunc(computePipeline->computeShader.base);

		return true;
	};

	auto destroy = [](int, void*)
	{
		return true;
	};
	auto changed = [](std::shared_ptr<ResourceBase const> ptr_)
	{
		return false;
	};
	auto save = [](std::shared_ptr<ResourceBase const> ptr_, ResourceManager::ISaver& writer_)
	{
		auto computePipeline = std::static_pointer_cast<ComputePipeline const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto RenderPipeline::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		Topology topology_,
		RenderPipelineFlags flags_,
		DynamicPipelineState dynamicPipelineState_,
		std::vector<BindingTableMemoryMapHandle> const& memoryMap_,
		std::vector<PushConstantRange> const& pushConstantRanges_,
		std::vector<SpecializationConstant> const& specializationConstants_,
		std::vector<SPIRVShaderHandle> const& shaders_, // vertex, fragment, geometry, tess control, tess eval
		RasterisationStateHandle rasterisationState_,
		RenderPassHandle renderPass_,
		ROPBlenderHandle ropBlender_,
		VertexInputHandle vertexInput_,
		ViewportHandle viewport_) -> RenderPipelineHandle
{
	assert(shaders_.size() > 0);

	size_t const memoryMapSize = sizeof(BindingTableMemoryMapHandle) * memoryMap_.size();
	size_t const pushSize = sizeof(PushConstantRange) * pushConstantRanges_.size();
	size_t const shaderSize = sizeof(SPIRVShaderHandle) * shaders_.size();
	size_t const specialSize = sizeof(SpecializationConstant) * specializationConstants_.size();
	size_t const dataSize = memoryMapSize + pushSize + shaderSize + specialSize;

	size_t const totalSize = sizeof(RenderPipeline) + dataSize;

	auto obj = (RenderPipeline*) malloc(totalSize);
	std::memset(obj, 0, totalSize);
	obj->sizeAndStageCount = totalSize;

	obj->dynamicPipelineState = dynamicPipelineState_;
	obj->inputTopology = topology_;
	obj->flags = flags_;

	obj->numBindingTableMemoryMaps = (uint8_t) memoryMap_.size();
	obj->numPushConstantRanges = (uint8_t) pushConstantRanges_.size();
	obj->numShaders = (uint8_t) shaders_.size();
	obj->numSpecializationConstants = (uint8_t)specializationConstants_.size();

	auto memorymapPtr = obj->getBindingTableMemoryMapHandles();
	std::memcpy(const_cast<BindingTableMemoryMapHandle*>(memorymapPtr), memoryMap_.data(), memoryMapSize);
	auto pushPtr = obj->getPushConstantRanges();
	std::memcpy(const_cast<PushConstantRange*>(pushPtr), pushConstantRanges_.data(), pushSize);
	auto shaderPtr = obj->getSPIRVShaderHandles();
	std::memcpy(const_cast<SPIRVShaderHandle*>(shaderPtr), shaders_.data(), shaderSize);
	auto specialPtr = obj->getSpecializationConstants();
	std::memcpy(const_cast<SpecializationConstant*>(specialPtr), specializationConstants_.data(), specialSize);

	obj->rasterisationState = rasterisationState_;
	obj->renderPass = renderPass_;
	obj->ropBlender = ropBlender_;
	obj->viewport = viewport_;
	obj->vertexInput = vertexInput_;

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);

}

} // end namespace