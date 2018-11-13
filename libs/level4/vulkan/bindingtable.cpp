#include "core/core.h"
#include "vulkan/api.h"
#include "render/bindingtable.h"
#include "vulkan/bindingtable.h"
#include "resourcemanager/resourceman.h"
#include "vulkan/types.h"
#include "vulkan/device.h"
#include "texture.h"

namespace Vulkan {

auto BindingTableMemoryMap::RegisterResourceHandler(ResourceManager::ResourceMan& rm_,
													std::weak_ptr<Device> device_) -> void
{
	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto bindingTableMM = std::static_pointer_cast<Render::BindingTableMemoryMap>(ptr_);
		auto vulkanBindingtableMM = bindingTableMM->getStage<Vulkan::BindingTableMemoryMap, false>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		createInfo.bindingCount = bindingTableMM->numBindings;

		std::vector<VkDescriptorSetLayoutBinding> bindings(bindingTableMM->numBindings);
		for(auto i = 0u; i < bindingTableMM->numBindings; ++i)
		{
			VkDescriptorSetLayoutBinding& vkbinding = bindings[i];
			Render::BindingLayout const* binding = bindingTableMM->getBindingLayouts();
			vkbinding.descriptorType = from(binding[i].type);
			vkbinding.binding = i;
			vkbinding.descriptorCount = binding[i].count;
			vkbinding.stageFlags = from(binding[i].shaderAccess);
			vkbinding.pImmutableSamplers = nullptr; // not sure to support yet

		}
		vulkanBindingtableMM->layout = device->createDescriptorSetLayout(createInfo);
		return true;
	};


	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		auto bindingTable = (Render::BindingTableMemoryMap*) ptr_;
		auto vulkanBindingTable = bindingTable->getStage<Vulkan::BindingTableMemoryMap>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		return true;
	};

	s_stage = rm_.registerNextHandler(Render::BindingTableMemoryMapId,
									  {sizeof(Vulkan::BindingTableMemoryMap), registerFunc, deleteFunc});
}

auto BindingTable::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface resolver_, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto bindingTable = std::static_pointer_cast<Render::BindingTable>(ptr_);
		auto vulkanBindingTable = bindingTable->getStage<Vulkan::BindingTable>(stage_);

		auto device = device_.lock();
		if(!device) return false;
		vulkanBindingTable->device = device->getVkDevice();

		VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		allocateInfo.descriptorPool = device->getDescriptorPool();
		allocateInfo.descriptorSetCount = bindingTable->numMemoryMaps;
		std::vector<VkDescriptorSetLayout> layouts(bindingTable->numMemoryMaps);
		for(auto i = 0u; i < bindingTable->numMemoryMaps; ++i)
		{
			auto memoryMap = bindingTable->getMemoryMaps()[i].acquire<Render::BindingTableMemoryMap>();
			auto vulkanMemoryMap = memoryMap->getStage<BindingTableMemoryMap>(stage_);
			layouts[i] = vulkanMemoryMap->layout;
		}
		allocateInfo.pSetLayouts = layouts.data();

		vulkanBindingTable->descriptorSets.resize(bindingTable->numMemoryMaps);
		vulkanBindingTable->memoryMaps.resize(bindingTable->numMemoryMaps);

		// we need to keep the memory maps in memory and easy to get to, so acquire here
		for(auto i = 0u; i < bindingTable->numMemoryMaps; ++i)
		{
			vulkanBindingTable->memoryMaps[i] = bindingTable->getMemoryMaps()[i].acquire<Render::BindingTableMemoryMap>();
		}

		device->allocDescriptorSet(allocateInfo, vulkanBindingTable->descriptorSets);
		vulkanBindingTable->vtable = &device->descriptorSetVkVTable;

		return true;
	};


	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		auto bindingTable = (Render::BindingTable*) ptr_;
		auto vulkanBindingTable = bindingTable->getStage<Vulkan::BindingTable>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		device->freeDescriptorSet(vulkanBindingTable->descriptorSets);

		return true;
	};

	s_stage = rm_.registerNextHandler(Render::BindingTableId,
									  {sizeof(Vulkan::BindingTable), registerFunc, deleteFunc});
}

auto BindingTable::update(uint8_t memoryMapIndex_,
						  uint32_t bindingIndex_,
						  std::vector<Render::TextureHandle> const& textures_) -> void
{
	auto memoryMap = memoryMaps[memoryMapIndex_];

	VkWriteDescriptorSet writer { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writer.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; // TODO
	writer.descriptorCount = (uint32_t)textures_.size();
	writer.dstBinding = bindingIndex_;
	writer.dstSet = descriptorSets[memoryMapIndex_];

	std::vector<VkDescriptorImageInfo> imageInfos(textures_.size());
	for(auto i = 0u; i < textures_.size(); ++i)
	{
		auto texture = textures_[i].acquire<Render::Texture>();
		auto vulkanTexture = texture->getStage<Texture>(Texture::s_stage);
		imageInfos[i].imageView = vulkanTexture->imageView;
		imageInfos[i].imageLayout = vulkanTexture->imageLayout;
	}
	writer.pImageInfo = imageInfos.data();

	vkUpdateDescriptorSets(
			1,
			&writer,
			0,
			nullptr);

}


} // end namespace Vulkan