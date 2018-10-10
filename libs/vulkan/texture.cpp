#include "core/core.h"
#include "vulkan/api.h"
#include "vulkan/texture.h"
#include "vulkan/system.h"
#include "vulkan/vkfcracker.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {

auto Texture::RegisterResourceHandler( ResourceManager::ResourceMan& rm_, Device::WeakPtr device_ ) -> void
{
	Vulkan::System* system = Vulkan::System::Global;

	rm_.registerNextResourceHandler( Id, { sizeof(Vulkan::Texture),
		 [device_]( uint16_t, uint16_t, int stage, std::shared_ptr<void> ptr_ ) -> bool
		 {
			 auto cpuTexture = std::static_pointer_cast<Render::Texture>( ptr_ );
			 auto vkTexture = cpuTexture->getExtraMemPtr<Vulkan::Texture>(stage);

			 auto device = device_.lock();
			 if(!device) return false;

			 VkImageType const type = cpuTexture->is3D() ? VK_IMAGE_TYPE_3D :
									  cpuTexture->is2D() ? VK_IMAGE_TYPE_2D :
									  VK_IMAGE_TYPE_1D;

			 VkImageUsageFlags usageflags = 0;
			 usageflags |= cpuTexture->canBeTransferSrc() ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0;
			 usageflags |= cpuTexture->canBeTransferDst() ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0;
			 usageflags |= cpuTexture->canBeShaderRead() ? VK_IMAGE_USAGE_SAMPLED_BIT : 0;
			 if(cpuTexture->canBeRopRead() || cpuTexture->canBeRopWrite())
			 {
			 	if(Render::GtfCracker::isDepthStencil(cpuTexture->format))
				{
			 		usageflags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				} else
				{
					usageflags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
			 }

			 VkImageCreateInfo imageCreateInfo = {
					 VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
					 nullptr,
					 cpuTexture->isCubeMap() ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
					 type,
					 VkfCracker::fromGeneric(cpuTexture->format),
					 { cpuTexture->width, cpuTexture->height, cpuTexture->depth},
					 cpuTexture->mipLevels,
					 cpuTexture->slices,
					 (VkSampleCountFlagBits)cpuTexture->samples,
					 VK_IMAGE_TILING_OPTIMAL,
					 usageflags,
					 VK_SHARING_MODE_EXCLUSIVE,
					 0,
					 nullptr,
					 VK_IMAGE_LAYOUT_UNDEFINED
			 };
			 VmaAllocationCreateInfo gpuAllocInfo;
			 gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			 gpuAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
			 VmaAllocationInfo info;
			 device->createImage(imageCreateInfo, gpuAllocInfo, info);
			 if(info.pMappedData != nullptr)
			 {
			 	// is host mappable, so just memcpy/memset it directly
			 	if(cpuTexture->flags & Render::Texture::InitZeroFlag)
				{
			 		std::memset(info.pMappedData, 0, info.size);
				} else
				{
			 		// mem copy
//			 		if(cpuTexture->)
				}
			 } else
			 {
			 	// no host mappable, so create a temp transfer buffer and
			 	// dma it up
			 }

			 return true;
		 },
		 [device_]( int stage, void * ptr_) -> bool
		 {
			 Vulkan::Texture* vkTexture = (Vulkan::Texture*)(((uintptr_t*)ptr_)[-(stage+1)]);

			 auto device = device_.lock();
			 if(!device) return false;

			 device->destroyImage(vkTexture->image);

			 return true;
		 }
	} );
}
} // end namespace