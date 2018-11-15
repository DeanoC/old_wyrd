#include "core/core.h"
#include "vulkan/api.h"
#include "resourcemanager/resourceman.h"
#include "render/bindingtable.h"
#include "render/buffer.h"
#include "vulkan/bindingtable.h"
#include "vulkan/buffer.h"
#include "vulkan/types.h"
#include "vulkan/device.h"
#include "vulkan/texture.h"
#include "vulkan/sampler.h"

namespace Vulkan {

auto BindingTableMemoryMap::RegisterResourceHandler(ResourceManager::ResourceMan& rm_,
													std::weak_ptr<Device> device_) -> void
{
	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  std::shared_ptr<ResourceManager::ResourceBase> ptr_) -> bool
	{
		auto bindingTableMM = std::static_pointer_cast<Render::BindingTableMemoryMap>(ptr_);
		auto vulkanBindingtableMM = bindingTableMM->getStage<Vulkan::BindingTableMemoryMap, false>(stage_);
		new(vulkanBindingtableMM) BindingTableMemoryMap();
		auto device = device_.lock();
		if(!device) return false;

		VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		createInfo.bindingCount = bindingTableMM->numBindingLayouts;

		std::vector<VkDescriptorSetLayoutBinding> bindings(bindingTableMM->numBindingLayouts);
		for(auto i = 0u; i < bindingTableMM->numBindingLayouts; ++i)
		{
			VkDescriptorSetLayoutBinding& vkbinding = bindings[i];
			Render::BindingLayout const* binding = bindingTableMM->getBindingLayouts();
			vkbinding.descriptorType = from(binding[i].type);
			vkbinding.binding = i;
			vkbinding.descriptorCount = binding[i].count;
			vkbinding.stageFlags = from(binding[i].shaderAccess);
			vkbinding.pImmutableSamplers = nullptr; // not sure to support yet

		}
		createInfo.pBindings = bindings.data();

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
								  std::shared_ptr<ResourceManager::ResourceBase> ptr_) -> bool
	{
		auto bindingTable = std::static_pointer_cast<Render::BindingTable>(ptr_);
		auto vulkanBindingTable = bindingTable->getStage<Vulkan::BindingTable,false>(stage_);
		new(vulkanBindingTable) BindingTable();

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
			Render::TextureHandle const& texture_) -> void
{
	auto memoryMap = memoryMaps[memoryMapIndex_];
	assert(bindingIndex_ < memoryMap->numBindingLayouts);
	Render::BindingLayout const& layout = memoryMap->getBindingLayouts()[bindingIndex_];

	VkWriteDescriptorSet writer { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writer.descriptorType = from(layout.type);
	writer.descriptorCount = 1;
	writer.dstBinding = bindingIndex_;
	writer.dstSet = descriptorSets[memoryMapIndex_];
	assert(writer.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

	VkDescriptorImageInfo imageInfo{};
	auto texture = texture_.acquire<Render::Texture>();
	auto vulkanTexture = texture->getStage<Texture>(Texture::s_stage);
	imageInfo.imageView = vulkanTexture->imageView;
	imageInfo.imageLayout = vulkanTexture->imageLayout;

	writer.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(
			1,
			&writer,
			0,
			nullptr);
}

auto BindingTable::update(uint8_t memoryMapIndex_,
	uint32_t bindingIndex_,
	Render::BufferHandle const& buffer_) -> void
{
	auto memoryMap = memoryMaps[memoryMapIndex_];
	assert(bindingIndex_ < memoryMap->numBindingLayouts);
	Render::BindingLayout const& layout = memoryMap->getBindingLayouts()[bindingIndex_];

	VkWriteDescriptorSet writer{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writer.descriptorType = from(layout.type);
	writer.descriptorCount = 1;
	writer.dstBinding = bindingIndex_;
	writer.dstSet = descriptorSets[memoryMapIndex_];
	assert(writer.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	VkDescriptorBufferInfo bufferInfo{};
	auto buffer = buffer_.acquire<Render::Buffer>();
	auto vulkanBuffer = buffer->getStage<Buffer>(Buffer::s_stage);
	bufferInfo.buffer = vulkanBuffer->buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = buffer->sizeInBytes;
	writer.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(
		1,
		&writer,
		0,
		nullptr);
}


auto BindingTable::update(uint8_t memoryMapIndex_,
			uint32_t bindingIndex_,
			Render::SamplerHandle const& sampler_) -> void
{
	auto memoryMap = memoryMaps[memoryMapIndex_];
	assert(bindingIndex_ < memoryMap->numBindingLayouts);
	Render::BindingLayout const& layout = memoryMap->getBindingLayouts()[bindingIndex_];

	VkWriteDescriptorSet writer { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writer.descriptorType = from(layout.type);
	writer.descriptorCount = 1;
	writer.dstBinding = bindingIndex_;
	writer.dstSet = descriptorSets[memoryMapIndex_];
	assert(writer.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER);

	VkDescriptorImageInfo imageInfo{};
	auto sampler = sampler_.acquire<Render::Sampler>();
	auto vulkanSampler = sampler->getStage<Sampler>(Sampler::s_stage);

	imageInfo.sampler = vulkanSampler->sampler;

	writer.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(
			1,
			&writer,
			0,
			nullptr);
}

auto BindingTable::update(uint8_t memoryMapIndex_,
			uint32_t bindingIndex_,
			Render::TextureHandle const& texture_,
			Render::SamplerHandle const& sampler_) -> void
{
	auto memoryMap = memoryMaps[memoryMapIndex_];
	assert(bindingIndex_ < memoryMap->numBindingLayouts);
	Render::BindingLayout const& layout = memoryMap->getBindingLayouts()[bindingIndex_];

	VkWriteDescriptorSet writer { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writer.descriptorType = from(layout.type);
	writer.descriptorCount = 1;
	writer.dstBinding = bindingIndex_;
	writer.dstSet = descriptorSets[memoryMapIndex_];
	assert(writer.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	VkDescriptorImageInfo imageInfo{};
	auto texture = texture_.acquire<Render::Texture>();
	auto sampler = sampler_.acquire<Render::Sampler>();
	auto vulkanTexture = texture->getStage<Texture>(Texture::s_stage);
	auto vulkanSampler = sampler->getStage<Sampler>(Sampler::s_stage);

	imageInfo.imageView = vulkanTexture->imageView;
	imageInfo.imageLayout = vulkanTexture->imageLayout;
	imageInfo.sampler = vulkanSampler->sampler;

	writer.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(
			1,
			&writer,
			0,
			nullptr);
}

auto BindingTable::update(uint8_t memoryMapIndex_,
						  uint32_t bindingIndex_,
						  std::vector<Render::TextureHandle> const& textures_) -> void
{
	auto memoryMap = memoryMaps[memoryMapIndex_];
	assert(bindingIndex_ < memoryMap->numBindingLayouts);
	Render::BindingLayout const& layout = memoryMap->getBindingLayouts()[bindingIndex_];

	assert(layout.count == (uint32_t) textures_.size());

	VkWriteDescriptorSet writer { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writer.descriptorType = from(layout.type);
	writer.descriptorCount = layout.count;
	writer.dstBinding = bindingIndex_;
	writer.dstSet = descriptorSets[memoryMapIndex_];

	std::vector<VkDescriptorImageInfo> imageInfos(textures_.size());
	for(auto i = 0u; i < textures_.size(); ++i)
	{
		auto texture = textures_[i].acquire<Render::Texture>();
		auto vulkanTexture = texture->getStage<Texture>(Texture::s_stage);
		imageInfos[i] = {};
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