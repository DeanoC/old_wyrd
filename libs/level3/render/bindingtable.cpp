#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/resourceman.h"
#include "binny/writehelper.h"
#include "render/bindingtable.h"

namespace Render {
auto BindingTableMemoryMap::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

		auto bindingTableMemoryMap = std::static_pointer_cast<BindingTableMemoryMap>(ptr_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

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
		auto bindingTableMemoryMap = std::static_pointer_cast<BindingTableMemoryMap const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto BindingTable::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

		auto bindingTable = std::static_pointer_cast<BindingTable>(ptr_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

		for(auto i = 0u; i < bindingTable->numMemoryMaps; ++i)
		{
			resolverFunc(bindingTable->getMemoryMaps()[i].base);
		}

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
		auto bindingTable = std::static_pointer_cast<BindingTable const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto BindingTableMemoryMap::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		std::vector<BindingLayout> const& bindingLayouts_) -> BindingTableMemoryMapHandle
{
	assert(bindingLayouts_.size() < (1 << (sizeof(BindingTableMemoryMap::numBindingLayouts) * 8)));

	size_t const bindSize = sizeof(BindingLayout) * bindingLayouts_.size();
	size_t const dataSize = bindSize;

	size_t const totalSize = Core::alignTo(sizeof(BindingTableMemoryMap) + dataSize, 8);
	auto obj = (BindingTableMemoryMap*) malloc(totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numBindingLayouts = (uint8_t) bindingLayouts_.size();

	auto bindPtr = obj->getBindingLayouts();
	std::memcpy(const_cast<BindingLayout*>(bindPtr), bindingLayouts_.data(), bindSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

auto BindingTable::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		std::vector<BindingTableMemoryMapHandle> const& bindingTables) -> BindingTableHandle
{
	assert(bindingTables.size() < (1 << (sizeof(BindingTable::numMemoryMaps) * 8)));

	size_t const dataSize = sizeof(BindingTableMemoryMapHandle) * bindingTables.size();
	size_t const totalSize = Core::alignTo(sizeof(BindingTable) + dataSize, 8);
	auto obj = (BindingTable*) malloc(totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numMemoryMaps = (uint8_t) bindingTables.size();

	uint8_t* dataPtr = ((uint8_t*) (obj + 1));
	std::memcpy(dataPtr, bindingTables.data(), dataSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}
} // end namespace