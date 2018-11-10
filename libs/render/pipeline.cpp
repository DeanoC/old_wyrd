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

		for(auto i = 0u; i < 5; ++i)
		{
			resolverFunc(renderPipeline->shaders[i].base);

		}

		return true;
	};

	auto destroy = [](int, void*)
	{
		return true;
	};
	auto changed = [](ResourceBase::ConstPtr ptr_)
	{
		return false;
	};
	auto save = [](ResourceBase::ConstPtr ptr_, ResourceManager::ISaver& writer_)
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
	auto changed = [](ResourceBase::ConstPtr ptr_)
	{
		return false;
	};
	auto save = [](ResourceBase::ConstPtr ptr_, ResourceManager::ISaver& writer_)
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
		std::vector<BindingTableMemoryMapHandle> const& memoryMap_,
		std::vector<SPIRVShaderHandle> const& shaders_, // vertex, fragment, geometry, tess control, tess eval
		RasterisationStateHandle rasterisationState_,
		RenderPassHandle renderPass_,
		ROPBlenderHandle ropBlender_,
		ViewportHandle viewport_,
		VertexInputHandle vertexInput_) -> RenderPipelineHandle
{
	assert(shaders_.size() > 0);

	size_t const dataSize = sizeof(BindingTableMemoryMapHandle) * memoryMap_.size();
	size_t const totalSize = sizeof(RenderPipeline) + dataSize;

	auto obj = (RenderPipeline*) malloc(totalSize);
	std::memset(obj, 0, totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numShaders = (uint8_t) shaders_.size();
	obj->numBindingTableMemoryMaps = (uint8_t) memoryMap_.size();
	obj->inputTopology = topology_;
	obj->flags = flags_;
	std::memcpy(obj + 1, memoryMap_.data(), dataSize);

	for(auto i = 0u; i < 5; ++i)
	{
		if(i < shaders_.size())
		{
			obj->shaders[i] = shaders_[i];
		} else
		{
			obj->shaders[i] = {};
		}
	}

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