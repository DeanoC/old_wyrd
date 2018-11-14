#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/resourceman.h"
#include "binny/writehelper.h"
#include "render/buffer.h"

namespace Render {
auto Buffer::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;

		auto buffer = std::static_pointer_cast<Buffer>(ptr_);
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
		auto buffer = std::static_pointer_cast<Buffer const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto Buffer::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		BufferFlags flags_,
		uint64_t sizeInBytes_,
		uint8_t* data_) -> BufferHandle
{
	using namespace Core::bitmask;
	bool noInit = false;
	size_t dataSize = sizeInBytes_;
	if(data_ == nullptr || test_any(flags_, BufferFlags::InitZero | BufferFlags::NoInit))
	{
		dataSize = 0;
		noInit = true;
		// if we don't have any data and not zero init set the noinit flag
		if(data_ == nullptr && !test_equal(flags_, BufferFlags::InitZero))
		{
			flags_ |= BufferFlags::NoInit;
		}
	}

	size_t const totalSize = Core::alignTo(sizeof(Buffer) + dataSize, 8);
	auto obj = (Buffer*) malloc(totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->flags = flags_;
	if(noInit && !test_equal(flags_, BufferFlags::InitZero))
	{
		obj->flags = obj->flags | BufferFlags::NoInit;
	}
	obj->sizeInBytes = sizeInBytes_;
	if(!noInit)
	{
		std::memcpy(obj + 1, data_, sizeInBytes_);
	}

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

auto Buffer::implicitDataChanged() const
{
	assert(Core::bitmask::test_equal(flags, BufferFlags::CPUDynamic));

}

} // end namespace