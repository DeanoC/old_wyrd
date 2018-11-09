#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/resourcename.h"
#include "binny/writehelper.h"
#include "render/viewport.h"

namespace Render {
auto Viewport::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_,
				   uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto viewport = std::static_pointer_cast<Viewport>(ptr_);

		if(viewport->numViewports == 0) return false;

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
		auto viewport = std::static_pointer_cast<Viewport const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);

}

auto Viewport::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		std::vector<ViewportAndScissor> const& viewports_) -> ViewportHandle
{
	size_t const dataSize = sizeof(ViewportAndScissor) * viewports_.size();
	size_t const totalSize = sizeof(Viewport) + dataSize;

	auto obj = (Viewport*) malloc(totalSize);
	std::memset(obj, 0, totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numViewports = (uint8_t) viewports_.size();
	std::memcpy(obj + 1, viewports_.data(), dataSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

} // end namespace