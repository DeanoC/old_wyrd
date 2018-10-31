#include "core/core.h"
#include "vulkan/api.h"
#include "vulkan/texture.h"
#include "vulkan/system.h"
#include "vulkan/vkfcracker.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {

auto Texture::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	Vulkan::System* system = Vulkan::System::Global;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto cpuTexture = std::static_pointer_cast<Render::Texture>(ptr_);
		auto vulkanTexture = cpuTexture->getStage<Vulkan::Texture>(stage_);
		vulkanTexture->cpuTexture = cpuTexture.get();

		auto device = device_.lock();
		if(!device) return false;

		VkImageType const type = cpuTexture->is3D() ? VK_IMAGE_TYPE_3D :
								 cpuTexture->is2D() ? VK_IMAGE_TYPE_2D :
								 VK_IMAGE_TYPE_1D;

		if(cpuTexture->slices > 1)
		{
			assert(cpuTexture->is3D() == false);
			vulkanTexture->imageViewType =
					cpuTexture->isCubeMap() ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY :
					cpuTexture->is2D() ? VK_IMAGE_VIEW_TYPE_2D_ARRAY :
					VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		} else
		{
			vulkanTexture->imageViewType =
					cpuTexture->isCubeMap() ? VK_IMAGE_VIEW_TYPE_CUBE :
					cpuTexture->is3D() ? VK_IMAGE_VIEW_TYPE_3D :
					cpuTexture->is2D() ? VK_IMAGE_VIEW_TYPE_2D :
					VK_IMAGE_VIEW_TYPE_1D;
		}


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

		VkImageCreateInfo imageCreateInfo{
				VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				nullptr,
				cpuTexture->isCubeMap() ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
				type,
				VkfCracker::fromGeneric(cpuTexture->format),
				{cpuTexture->width, cpuTexture->height, cpuTexture->depth},
				cpuTexture->mipLevels,
				cpuTexture->slices,
				(VkSampleCountFlagBits) cpuTexture->samples,
				VK_IMAGE_TILING_OPTIMAL,
				usageflags,
				VK_SHARING_MODE_EXCLUSIVE,
				0,
				nullptr,
				VK_IMAGE_LAYOUT_UNDEFINED
		};
		VmaAllocationCreateInfo gpuAllocInfo{};
		gpuAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		gpuAllocInfo.pool = VK_NULL_HANDLE;
		gpuAllocInfo.pUserData = nullptr;

		VmaAllocationInfo info;
		auto[image, alloc] = device->createImage(imageCreateInfo, gpuAllocInfo, info);
		vulkanTexture->image = image;
		vulkanTexture->allocation = alloc;

		if(info.pMappedData != nullptr)
		{
			// is host mappable, so just memcpy/memset it directly
			if(cpuTexture->flags & Render::Texture::InitZeroFlag)
			{
				for(auto i = 0u; i < cpuTexture->width * cpuTexture->height; ++i)
				{
					((uint32_t*) info.pMappedData)[i] = 0xFF0000FF;
				}
				//				std::memset(info.pMappedData, 0x0, info.size);
			} else
			{
				auto image = cpuTexture->imageHandle.acquire<Render::GenericImage>();
				assert(image);

				// mem copy
				std::memcpy(info.pMappedData, image->data(), image->dataSize);
			}

		} else
		{
			// no host mappable, so create a temp transfer buffer and
			// dma it up
			assert(false);
		}
		// default range
		vulkanTexture->entireRange.aspectMask = Render::GtfCracker::isDepth(cpuTexture->format)
												? VK_IMAGE_ASPECT_DEPTH_BIT
												: 0;
		vulkanTexture->entireRange.aspectMask = Render::GtfCracker::isStencil(cpuTexture->format)
												? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
		if(vulkanTexture->entireRange.aspectMask == 0)
		{
			vulkanTexture->entireRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		vulkanTexture->entireRange.baseMipLevel = 0;
		vulkanTexture->entireRange.levelCount = VK_REMAINING_MIP_LEVELS;
		vulkanTexture->entireRange.baseArrayLayer = 0;
		vulkanTexture->entireRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		// default view
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.format = imageCreateInfo.format;
		imageViewCreateInfo.image = vulkanTexture->image;
		imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
										  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
		imageViewCreateInfo.subresourceRange = vulkanTexture->entireRange;
		imageViewCreateInfo.viewType = vulkanTexture->imageViewType;
		imageViewCreateInfo.flags = imageCreateInfo.flags;
		vulkanTexture->imageView = device->createImageView(imageViewCreateInfo);

		vulkanTexture->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		return true;
	};


	auto deleteFunc = [device_](int stage, void* ptr_) -> bool
	{
		Vulkan::Texture* vulkanTexture = (Vulkan::Texture*) (((uintptr_t*) ptr_)[-(stage + 1)]);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyImageView(vulkanTexture->imageView);
		device->destroyImage({vulkanTexture->image, vulkanTexture->allocation});

		return true;
	};

	s_stage = rm_.registerNextResourceHandler(Id, {sizeof(Vulkan::Texture), registerFunc, deleteFunc});
}

} // end namespace