#include "core/core.h"
#include "render/sampler.h"
#include "vulkan/sampler.h"
#include "vulkan/device.h"
#include "vulkan/types.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {
auto Sampler::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	using namespace Core::bitmask;

	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  std::shared_ptr<ResourceManager::ResourceBase> ptr_) -> bool
	{
		auto sampler = std::static_pointer_cast<Render::Sampler>(ptr_);
		auto vulkanSampler = sampler->getStage<Vulkan::Sampler, false>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		VkSamplerCreateInfo createInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
		createInfo.minFilter = from(sampler->magFilter);
		createInfo.minFilter = from(sampler->minFilter);
		createInfo.mipmapMode = (sampler->mipFilter == Render::Filter::Nearest) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.addressModeU = from(sampler->uAddressMode);
		createInfo.addressModeV = from(sampler->vAddressMode);
		createInfo.addressModeW = from(sampler->wAddressMode);
		createInfo.compareOp = from(sampler->compareOp);
		createInfo.anisotropyEnable = sampler->isAnistropyEnabled();
		createInfo.compareEnable = sampler->isCompareEnabled();
		createInfo.mipLodBias = sampler->lodBias;
		createInfo.maxAnisotropy = sampler->maxAnistropy;
		createInfo.minLod = sampler->minLod;
		createInfo.maxLod = sampler->maxLod;

		vulkanSampler->sampler = device->createSampler(createInfo);

		return true;
	};

	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		Render::Sampler* sampler = (Render::Sampler*) ptr_;
		Vulkan::Sampler* vulkanSampler = sampler->getStage<Vulkan::Sampler>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->destroySampler(vulkanSampler->sampler);

		return true;
	};

	s_stage = rm_.registerNextHandler(Render::Sampler::Id,
									  {sizeof(Vulkan::Sampler), registerFunc, deleteFunc});
}

}
