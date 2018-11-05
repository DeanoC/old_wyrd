#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "binny/writehelper.h"
#include "render/rendertarget.h"

namespace Render {
auto RenderTarget::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_,
				   uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

		auto renderTarget = std::static_pointer_cast<RenderTarget>(ptr_);
		auto[getRMFunc, resolverFunc] = resolver_;

		resolverFunc(renderTarget->renderPassHandle.base);
		for(auto i = 0u; i < RenderPass::MaxTargets; ++i)
		{
			resolverFunc(renderTarget->targetHandles[i].base);
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
		auto texture = std::static_pointer_cast<RenderPass const>(ptr_);

		return true;
	};

	rm_.registerResourceHandler(Id, {0, load, destroy}, changed, save);

}

} // end namespace