#include "core/core.h"
#include "vulkan/api.h"
#include "render/bindingtable.h"
#include "vulkan/bindingtable.h"
#include "vulkan/system.h"
#include "resourcemanager/resourceman.h"

namespace Vulkan {

auto BindingTable::RegisterResourceHandler(ResourceManager::ResourceMan& rm_, Device::WeakPtr device_) -> void
{
	auto registerFunc = [device_](int stage_, ResourceManager::ResolverInterface, uint16_t, uint16_t,
								  ResourceManager::ResourceBase::Ptr ptr_) -> bool
	{
		auto bindingTable = std::static_pointer_cast<Render::BindingTable>(ptr_);
		auto vulkanBindingtable = bindingTable->getStage<Vulkan::BindingTable>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		/*		VkDescriptorSetLayout layout;
				layout.
				VkDescriptorPoolSize poolSize;
				poolSize.descriptorCount = m;
				poolSize.type = VkDescriptorType;
				VkDescriptorPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
				poolCreateInfo.maxSets = 1;
				poolCreateInfo.poolSizeCount = n;
				poolCreateInfo.
		*/
		return true;
	};


	auto deleteFunc = [device_](int stage_, void* ptr_) -> bool
	{
		auto bindingTable = (Render::BindingTable*) ptr_;
		auto vulkanBindingTable = bindingTable->getStage<Vulkan::BindingTable>(stage_);

		auto device = device_.lock();
		if(!device) return false;

		return true;
	};

	s_stage = rm_.registerNextResourceHandler(Render::BindingTableId,
											  {sizeof(Vulkan::BindingTable), registerFunc, deleteFunc});

}

}