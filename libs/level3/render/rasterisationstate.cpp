#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/resourceman.h"
#include "binny/writehelper.h"
#include "render/rasterisationstate.h"

namespace Render {
auto RasterisationState::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

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
		auto rasterState = std::static_pointer_cast<RasterisationState const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto RasterisationState::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		RasterisationStateFlags flags_,
		CullMode cullMode_,
		CompareOp depthCompare_,
		FillMode fillMode_,
		FrontFace frontFace_,
		SampleCounts sampleCount_,
		StencilState frontStencil_,
		StencilState backStencil_,
		float lineWidth_,
		float minDepthBounds_,
		float maxDepthBounds_,
		float minSampleShadingRate_,
		DepthBias depthBias_,
		uint32_t patchControlPointsCount_,
		uint64_t sampleMask_) -> RasterisationStateHandle
{
	RasterisationState obj{};
	obj.sizeAndStageCount = sizeof(RasterisationState);
	obj.flags = flags_;
	obj.cullMode = cullMode_;
	obj.depthCompare = depthCompare_;
	obj.fillMode = fillMode_;
	obj.frontFace = frontFace_;
	obj.sampleCount = sampleCount_;
	obj.frontStencil = frontStencil_;
	obj.backStencil = backStencil_;
	obj.lineWidth = lineWidth_;
	obj.minDepthBounds = minDepthBounds_;
	obj.maxDepthBounds = maxDepthBounds_;
	obj.minSampleShadingRate = minSampleShadingRate_;
	obj.depthBias = depthBias_;
	obj.patchControlPointsCount = patchControlPointsCount_;
	obj.sampleMask = sampleMask_;

	rm_->placeInStorage(name_, obj);
	return rm_->openByName<Id>(name_);
}

} // end namespace