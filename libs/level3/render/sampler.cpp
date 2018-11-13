#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/resourcename.h"
#include "binny/writehelper.h"
#include "render/sampler.h"

namespace Render {
auto Sampler::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_,
				   uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto sampler = std::static_pointer_cast<Sampler>(ptr_);

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
		auto sampler = std::static_pointer_cast<Sampler const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);

}

auto Sampler::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		Filter magFilter_,
		Filter minFilter_,
		Filter mipFilter_,
		SamplerAddressMode uAddressMode_,
		SamplerAddressMode vAddressMode_,
		SamplerAddressMode wAddressMode_,
		SamplerFlags flags_,
		float maxAnistropy_,
		float lodBias_,
		CompareOp compareOp_,
		float minLod_,
		float maxLod_) -> SamplerHandle
{
	size_t const dataSize = 0;
	size_t const totalSize = sizeof(Sampler) + dataSize;

	auto obj = (Sampler*) malloc(totalSize);
	std::memset(obj, 0, totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->magFilter = magFilter_;
	obj->minFilter = minFilter_;
	obj->mipFilter = mipFilter_;
	obj->uAddressMode = uAddressMode_;
	obj->vAddressMode = vAddressMode_;
	obj->wAddressMode = wAddressMode_;
	obj->flags = flags_;
	obj->maxAnistropy = maxAnistropy_;
	obj->lodBias = lodBias_;
	obj->compareOp = compareOp_;
	obj->minLod = minLod_;
	obj->maxLod  = maxLod_;
	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

} // end namespace