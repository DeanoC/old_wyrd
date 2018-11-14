#include "core/core.h"
#include "render/buffer.h"
#include "vulkan/buffer.h"
#include "vulkan/device.h"
#include "vulkan/vkfcracker.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {
auto Buffer::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  std::shared_ptr<ResourceManager::ResourceBase> ptr_) -> bool
	{
		auto buffer = std::static_pointer_cast<Render::Buffer const>(ptr_);
		auto vulkanBuffer = buffer->getStage<Vulkan::Buffer, false>(stage_);
		new(vulkanBuffer) Buffer();

		auto device = device_.lock();
		if(!device) return false;
		vulkanBuffer->weakDevice = device_;

		VkBufferUsageFlags usageflags = 0;
		usageflags |= buffer->canBeDMASrc() ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : 0;
		usageflags |= buffer->canBeDMADst() ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0;
		usageflags |= buffer->canBeReadByShader() ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;
		usageflags |= buffer->canBeWrittenByShader() ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByVertex() ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByIndex() ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByTexture() ? VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByIndirect() ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : 0;

		vulkanBuffer->createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		vulkanBuffer->createInfo.usage = usageflags;
		vulkanBuffer->createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vulkanBuffer->createInfo.size = buffer->sizeInBytes;

		VmaAllocationCreateInfo gpuAllocInfo{};
		if(buffer->canBeUpdatedByCPU())
		{
			gpuAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		} else
		{
			gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		VmaAllocationInfo info;
		auto[vkbuffer, alloc] = device->createBuffer(vulkanBuffer->createInfo, gpuAllocInfo, info);
		if(vkbuffer == VK_NULL_HANDLE) return false;
		if(alloc == nullptr) return false;
		vulkanBuffer->buffer = vkbuffer;
		vulkanBuffer->allocation = alloc;

		if(!test_equal(buffer->flags, Render::BufferFlags::NoInit))
		{
			if(test_equal(buffer->flags, Render::BufferFlags::InitZero))
			{
				device->fill(0x0, vulkanBuffer);
			} else
			{
				device->upload(buffer->getData(), buffer->sizeInBytes,  vulkanBuffer);
			}
		}

		return true;
	};

	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::Buffer* buffer = (Render::Buffer*) ptr_;
		Vulkan::Buffer* vulkanBuffer = buffer->getStage<Vulkan::Buffer>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyBuffer({vulkanBuffer->buffer, vulkanBuffer->allocation});

		return true;
	};

	s_stage = rm_.registerNextHandler(Render::BufferId,
									  {sizeof(Vulkan::Buffer), registerFunc, deleteFunc});
}

auto Buffer::update(uint8_t const* data_, uint64_t size_) -> void
{
	auto device = weakDevice.lock();
	assert(device);

	device->upload(data_, size_, this);

}
auto Buffer::map() -> void*
{
	auto device = weakDevice.lock();
	assert(device);

	return device->mapMemory(allocation);
}
auto Buffer::unmap() -> void
{
	auto device = weakDevice.lock();
	assert(device);

	device->unmapMemory(allocation);
	device->flushMemory(allocation);

}

}
