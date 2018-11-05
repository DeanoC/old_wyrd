#include "core/core.h"
#include "resourcemanager/resourceman.h"
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
		if(renderPass->numTargets >= RenderPass::MaxTargets) return false;

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
		auto texture = std::static_pointer_cast<RenderPass const>(ptr_);

		return true;
	};

	rm_.registerResourceHandler(Id, {0, load, destroy}, changed, save);

}

} // end namespace