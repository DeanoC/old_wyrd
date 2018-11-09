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
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

		if(renderTarget->renderExtent[0] == 0) return false;
		if(renderTarget->renderExtent[1] == 0) return false;
		if(renderTarget->numTargetTextures == 0) return false;

		resolverFunc(renderTarget->renderPassHandle.base);
		for(auto i = 0u; i < renderTarget->numTargetTextures; ++i)
		{
			resolverFunc(renderTarget->getTargetTextures()[i].base);
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

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);

}

auto RenderTarget::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		RenderPassHandle renderPassHandle_,
		std::vector<TextureHandle> const& targetTextures_,
		std::array<int32_t, 2> const& renderOffset_,
		std::array<uint32_t, 2> const& renderExtent_) -> RenderTargetHandle
{
	size_t const dataSize = sizeof(TextureHandle) * targetTextures_.size();
	size_t const totalSize = Core::alignTo(sizeof(RenderTarget) + dataSize, 8);
	RenderTarget* rt = (RenderTarget*) malloc(totalSize);
	rt->sizeAndStageCount = totalSize;
	rt->renderPassHandle = renderPassHandle_;
	std::memcpy(rt->renderOffset, renderOffset_.data(), sizeof(int32_t) * 2);
	std::memcpy(rt->renderExtent, renderExtent_.data(), sizeof(uint32_t) * 2);
	uint8_t* dataPtr = (uint8_t*) (rt + 1);
	TextureHandle* targetTextures = (TextureHandle*) dataPtr;
	for(auto i = 0u; i < targetTextures_.size(); ++i)
	{
		targetTextures[i] = targetTextures_[i];
	}

	rm_->placeInStorage(name_, *rt);
	free(rt);
	return rm_->openByName<Id>(name_);
}


} // end namespace