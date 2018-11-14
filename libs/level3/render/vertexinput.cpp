#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/resourcename.h"
#include "binny/writehelper.h"
#include "render/vertexinput.h"

namespace Render {
auto VertexInput::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_,
				   uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto vertexInput = std::static_pointer_cast<VertexInput>(ptr_);

		if(vertexInput->numVertexInputs == 0) return false;

		return true;
	};

	auto destroy = [](int, void*)
	{
		return true;
	};
	auto changed = [](std::shared_ptr<ResourceBase const> ptr_)
	{
		return false;
	};
	auto save = [](std::shared_ptr<ResourceBase const> ptr_, ResourceManager::ISaver& writer_)
	{
		auto vertexInput = std::static_pointer_cast<VertexInput const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);

}

auto VertexInput::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		std::vector<Input> const& inputs_) -> VertexInputHandle
{
	size_t const dataSize = sizeof(Input) * inputs_.size();
	size_t const totalSize = sizeof(VertexInput) + dataSize;

	auto obj = (VertexInput*) malloc(totalSize);
	std::memset(obj, 0, totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->numVertexInputs = (uint8_t) inputs_.size();
	std::memcpy(obj + 1, inputs_.data(), dataSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

auto VertexInput::getElementSize(uint8_t const index_) const -> uint32_t
{
	assert(index_ < numVertexInputs);
	auto const& input = getInputs()[index_];
	switch(input.type)
	{
		case VertexInputType::Float1:
			return sizeof(float) * 1;
		case VertexInputType::Float2:
			return sizeof(float) * 2;
		case VertexInputType::Float3:
			return sizeof(float) * 3;
		case VertexInputType::Float4:
			return sizeof(float) * 4;
		case VertexInputType::Byte4:
			return sizeof(uint32_t);
		default:
			assert(false);
			return ~0;
	}
}

auto VertexInput::getElementOffset(uint8_t const index_) const -> uint32_t
{
	assert(index_ < numVertexInputs);

	uint32_t offset = 0;
	for(auto i = 0u; i < index_; ++i)
	{
		offset += getElementSize(i);
	}
	return offset;
}

auto VertexInput::getStride() const -> uint32_t
{

	uint32_t stride = 0;
	for(auto i = 0u; i < numVertexInputs; ++i)
	{
		stride += getElementSize(i);
	}
	return stride;
}


} // end namespace