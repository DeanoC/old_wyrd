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

		resolverFunc(renderPipeline->vertexShader.base);
		resolverFunc(renderPipeline->fragmentShader.base);

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

	rm_.registerResourceHandler(Id, {0, load, destroy}, changed, save);
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

	rm_.registerResourceHandler(Id, {0, load, destroy}, changed, save);
}

auto RenderPipeline::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		Topology topology_,
		uint8_t flags_,
		SPIRVShaderHandle vertexShader_,
		SPIRVShaderHandle tesselationControlShader_,
		SPIRVShaderHandle tesselationEvalShader_,
		SPIRVShaderHandle geometryShader_,
		SPIRVShaderHandle fragmentShader_) -> RenderPipelineHandle
{
	RenderPipeline renderPipeline{};
	renderPipeline.stage0 = sizeof(RenderPipeline);
	renderPipeline.inputTopology = topology_;
	renderPipeline.flags = flags_;
	renderPipeline.vertexShader = vertexShader_;
	renderPipeline.tesselationControlShader = tesselationControlShader_;
	renderPipeline.tesselationEvalShader = tesselationEvalShader_;
	renderPipeline.geometryShader = geometryShader_;
	renderPipeline.fragmentShader = fragmentShader_;

	rm_->placeInStorage(name_, renderPipeline);
	return rm_->openResourceByName<Id>(name_);
}

} // end namespace