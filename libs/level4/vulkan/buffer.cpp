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
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto buffer = std::static_pointer_cast<Render::Buffer const>(ptr_);
		auto vulkanBuffer = buffer->getStage<Vulkan::Buffer, false>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		VkBufferUsageFlags usageflags = 0;
		usageflags |= buffer->canBeDMASrc() ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : 0;
		usageflags |= buffer->canBeDMADst() ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0;
		usageflags |= buffer->canBeReadByShader() ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;
		usageflags |= buffer->canBeWrittenByShader() ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByVertex() ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByIndex() ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByTexture() ? VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT : 0;
		usageflags |= buffer->canBeReadByIndirect() ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : 0;

		VkBufferCreateInfo createInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		createInfo.usage = usageflags;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.size = buffer->sizeInBytes;

		VmaAllocationCreateInfo gpuAllocInfo{};
		gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VmaAllocationInfo info;
		auto[vkbuffer, alloc] = device->createBuffer(createInfo, gpuAllocInfo, info);
		if(vkbuffer == VK_NULL_HANDLE) return false;
		if(alloc == nullptr) return false;
		vulkanBuffer->buffer = vkbuffer;
		vulkanBuffer->allocation = alloc;

		if(!test_equal(buffer->flags, Render::BufferFlags::NoInit))
		{
			if(test_equal(buffer->flags, Render::BufferFlags::InitZero))
			{
				device->fill(0x0, createInfo, buffer);
			} else
			{
				device->upload(buffer->getData(), buffer->sizeInBytes, createInfo, buffer);
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

}
