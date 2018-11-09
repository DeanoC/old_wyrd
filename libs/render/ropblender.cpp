#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/resourceman.h"
#include "binny/writehelper.h"
#include "render/ropblender.h"

namespace Render {
auto ROPBlender::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

		auto ropBlender = std::static_pointer_cast<ROPBlender>(ptr_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

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
		auto ropBlender = std::static_pointer_cast<ROPBlender const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto ROPBlender::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		std::vector<ROPTargetBlender> const& blenders_,
		std::array<float, 4> const& constants_,
		bool logicOpEnable_,
		ROPLogicOps logicOp_) -> ROPBlenderHandle
{
	assert(blenders_.size() < (1 << (sizeof(ROPBlender::numTargets) * 8)));

	size_t const dataSize = sizeof(ROPBlender) * blenders_.size();
	size_t const totalSize = Core::alignTo(sizeof(ROPBlender) + dataSize, 8);
	auto obj = (ROPBlender*) malloc(totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numTargets = (uint8_t) blenders_.size();
	obj->flags = logicOpEnable_ ? LogicEnableFlag : 0;
	std::memcpy(obj->constants, constants_.data(), sizeof(float) * 4);
	obj->logicOp = logicOp_;
	std::memcpy(obj + 1, blenders_.data(), dataSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

} // end namespace