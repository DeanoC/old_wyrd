#include "core/core.h"
#include "vulkan/api.h"
#include "render/texture.h"
#include "vulkan/texture.h"
#include "vulkan/vkfcracker.h"
#include "vulkan/encoder.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {

auto Texture::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface resolver_, uint16_t, uint16_t,
								  std::shared_ptr<ResourceManager::ResourceBase> ptr_) -> bool
	{
		auto texture = std::static_pointer_cast<Render::Texture>(ptr_);
		auto vulkanTexture = texture->getStage<Vulkan::Texture, false>(stage_);
		new(vulkanTexture) Vulkan::Texture{};
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

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
		usageflags |= texture->canBeReadByShader() ? VK_IMAGE_USAGE_SAMPLED_BIT : 0;
		if(texture->canBeReadByRop() || texture->canBeWrittenByRop())
		{
			if(Render::GtfCracker::isDepthStencil(texture->format))
			{
				usageflags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			} else
			{
				usageflags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}

		vulkanTexture->format = VkfCracker::fromGeneric(texture->format);

		VkImageCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		createInfo.flags = texture->isCubeMap() ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u;
		createInfo.imageType = type;
		createInfo.format = vulkanTexture->format;
		createInfo.extent = {texture->width, texture->height, texture->depth};
		createInfo.mipLevels = texture->mipLevels;
		createInfo.arrayLayers = texture->slices;
		createInfo.samples = (VkSampleCountFlagBits) texture->samples;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.usage = usageflags;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vulkanTexture->imageLayout = createInfo.initialLayout;
		vulkanTexture->createInfo = createInfo;

		VmaAllocationCreateInfo gpuAllocInfo{};
		gpuAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VmaAllocationInfo info;
		auto[image, alloc] = device->createImage(createInfo, gpuAllocInfo, info);
		if(image == VK_NULL_HANDLE) return false;
		if(alloc == nullptr) return false;
		vulkanTexture->image = image;
		vulkanTexture->allocation = alloc;
		std::string resourceName(resourceNameFunc().getResourceName());
		device->debugNameVkObject(*(uint64_t*) &image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, resourceName.c_str());

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
		VkImageViewCreateInfo imageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		imageViewCreateInfo.format = createInfo.format;
		imageViewCreateInfo.image = vulkanTexture->image;
		imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
										  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
		imageViewCreateInfo.subresourceRange = vulkanTexture->entireRange;
		imageViewCreateInfo.viewType = vulkanTexture->imageViewType;
		imageViewCreateInfo.flags = createInfo.flags;
		vulkanTexture->imageView = device->createImageView(imageViewCreateInfo);

		// now the gpu texture has been created, we may need to schedule 
		// a cpu -> gpu transfer to initalise it if required.
		if(!test_equal(texture->flags, Render::TextureFlags::NoInit))
		{
			if(test_equal(texture->flags, Render::TextureFlags::InitZero))
			{
				device->fill(0x0, vulkanTexture);
			}
			else
			{
				auto img = texture->imageHandle.acquire<Render::GenericImage>();
				assert(img);

				device->upload(img->data(), img->dataSize, vulkanTexture);
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

	s_stage = rm_.registerNextHandler(Render::TextureId, {sizeof(Vulkan::Texture), registerFunc, deleteFunc});
}

auto Texture::transitionToRenderTarget(std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	Encoder::Ptr encoder = std::static_pointer_cast<Encoder>(encoder_);
	VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	barrier.oldLayout = imageLayout;

	if(VkfCracker::isDepth(format) || VkfCracker::isStencil(format))
	{
		barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} else
	{
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	imageLayout = barrier.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = entireRange;
	barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	if(VkfCracker::isDepth(format) || VkfCracker::isStencil(format))
	{
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	} else
	{
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	encoder->textureBarrier(
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			barrier);
}

auto Texture::transitionToDMADest(std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	Encoder::Ptr encoder = std::static_pointer_cast<Encoder>(encoder_);
	VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	barrier.oldLayout = imageLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageLayout = barrier.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = entireRange;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	encoder->textureBarrier(
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			barrier);
}

auto Texture::transitionToShaderSrc(std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	Encoder::Ptr encoder = std::static_pointer_cast<Encoder>(encoder_);
	VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	barrier.oldLayout = imageLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageLayout = barrier.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = entireRange;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	encoder->textureBarrier(
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			barrier);
}

auto Texture::transitionToDMASrc(std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	Encoder::Ptr encoder = std::static_pointer_cast<Encoder>(encoder_);
	VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	barrier.oldLayout = imageLayout;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageLayout = barrier.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = entireRange;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	encoder->textureBarrier(
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			barrier);
}

} // end namespace