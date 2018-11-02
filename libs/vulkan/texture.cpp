#include "core/core.h"
#include "vulkan/api.h"
#include "vulkan/texture.h"
#include "vulkan/system.h"
#include "vulkan/vkfcracker.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {

auto Texture::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;
	Vulkan::System* system = Vulkan::System::Global;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto texture = std::static_pointer_cast<Render::Texture>(ptr_);
		auto vulkanTexture = texture->getStage<Vulkan::Texture>(stage_);
		vulkanTexture->cpuTexture = texture.get();

		auto device = device_.lock();
		if(!device) return false;

		VkImageType const type = texture->is3D() ? VK_IMAGE_TYPE_3D :
								 texture->is2D() ? VK_IMAGE_TYPE_2D :
								 VK_IMAGE_TYPE_1D;

		if(texture->slices > 1)
		{
			assert(texture->is3D() == false);
			vulkanTexture->imageViewType =
					texture->isCubeMap() ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY :
					texture->is2D() ? VK_IMAGE_VIEW_TYPE_2D_ARRAY :
					VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		} else
		{
			vulkanTexture->imageViewType =
					texture->isCubeMap() ? VK_IMAGE_VIEW_TYPE_CUBE :
					texture->is3D() ? VK_IMAGE_VIEW_TYPE_3D :
					texture->is2D() ? VK_IMAGE_VIEW_TYPE_2D :
					VK_IMAGE_VIEW_TYPE_1D;
		}


		VkImageUsageFlags usageflags = 0;
		usageflags |= texture->canBeDMASrc() ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0;
		usageflags |= texture->canBeDMADst() ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0;
		usageflags |= texture->canBeShaderRead() ? VK_IMAGE_USAGE_SAMPLED_BIT : 0;
		if(texture->canBeRopRead() || texture->canBeRopWrite())
		{
			if(Render::GtfCracker::isDepthStencil(texture->format))
			{
				usageflags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			} else
			{
				usageflags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}

		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.flags = texture->isCubeMap() ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u;
		createInfo.imageType = type;
		createInfo.format = VkfCracker::fromGeneric(texture->format);
		createInfo.extent = {texture->width, texture->height, texture->depth};
		createInfo.mipLevels = texture->mipLevels;
		createInfo.arrayLayers = texture->slices;
		createInfo.samples = (VkSampleCountFlagBits) texture->samples;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.usage = usageflags;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo gpuAllocInfo{};
		gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VmaAllocationInfo info;
		auto[image, alloc] = device->createImage(createInfo, gpuAllocInfo, info);
		if(image == VK_NULL_HANDLE) return false;
		if(alloc == nullptr) return false;
		vulkanTexture->image = image;
		vulkanTexture->allocation = alloc;

		// default range
		vulkanTexture->entireRange.aspectMask = Render::GtfCracker::isDepth(texture->format)
												? VK_IMAGE_ASPECT_DEPTH_BIT
												: 0;
		vulkanTexture->entireRange.aspectMask = Render::GtfCracker::isStencil(texture->format)
												? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
		if(vulkanTexture->entireRange.aspectMask == 0)
		{
			vulkanTexture->entireRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		vulkanTexture->entireRange.baseMipLevel = 0;
		vulkanTexture->entireRange.levelCount = texture->mipLevels;
		vulkanTexture->entireRange.baseArrayLayer = 0;
		vulkanTexture->entireRange.layerCount = texture->slices;

		// default view
		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.format = createInfo.format;
		imageViewCreateInfo.image = vulkanTexture->image;
		imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
										  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
		imageViewCreateInfo.subresourceRange = vulkanTexture->entireRange;
		imageViewCreateInfo.viewType = vulkanTexture->imageViewType;
		imageViewCreateInfo.flags = createInfo.flags;
		vulkanTexture->imageView = device->createImageView(imageViewCreateInfo);

		vulkanTexture->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// now the gpu texture has been created, we may need to schedule 
		// a cpu -> gpu transfer to initalise it if required.
		if(!bool(texture->flags & Render::TextureFlag::NoInit))
		{
			if(bool(texture->flags & Render::TextureFlag::InitZero))
			{
				device->fill(0xFF0000FF, createInfo, texture);
			}
			else
			{
				auto img = texture->imageHandle.acquire<Render::GenericImage>();
				assert(img);

				device->upload(img->data(), img->dataSize, createInfo, texture);
			}
		}

		return true;
	};


	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::Texture* texture = (Render::Texture*) ptr_;
		Vulkan::Texture* vulkanTexture = texture->getStage<Vulkan::Texture>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroyImageView(vulkanTexture->imageView);
		device->destroyImage({vulkanTexture->image, vulkanTexture->allocation});

		return true;
	};

	s_stage = rm_.registerNextResourceHandler(Id, {sizeof(Vulkan::Texture), registerFunc, deleteFunc});
}

} // end namespace