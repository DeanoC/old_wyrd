#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/resourcename.h"
#include "binny/writehelper.h"
#include "render/renderpass.h"

namespace Render {
auto RenderPass::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_,
				   uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto renderPass = std::static_pointer_cast<RenderPass>(ptr_);

		if(renderPass->numTargets == 0) return false;

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
		auto renderPass = std::static_pointer_cast<RenderPass const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);

}

auto RenderPass::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		std::vector<Target> const& targets_,
		std::vector<Math::Vector4> const& clearValues_) -> RenderPassHandle
{
	assert(targets_.size() == clearValues_.size());
	size_t const targetSize = sizeof(Target) * targets_.size();
	size_t const clearSize = sizeof(Math::Vector4) * clearValues_.size();
	size_t const dataSize = targetSize + clearSize;

	size_t const totalSize = sizeof(RenderPass) + dataSize;

	auto obj = (RenderPass*) malloc(totalSize);
	std::memset(obj, 0, totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numTargets = (uint8_t) targets_.size();
	std::memcpy(obj + 1, targets_.data(), dataSize);
	std::memcpy(const_cast<Math::Vector4*>(obj->getClearValues()), clearValues_.data(), clearSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);

	return rm_->openByName<Id>(name_);
}

} // end namespace