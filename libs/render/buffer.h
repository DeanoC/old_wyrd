#pragma once
#ifndef WYRD_RENDER_BUFFER_H
#define WYRD_RENDR_BUFFER_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resourcename.h"

namespace ResourceManager { class ResourceMan; }

namespace Render {

enum class BufferFlags : uint32_t
{
	NoInit = Core::Bit(0),
	InitZero = Core::Bit(1),
	Usage = UsageMask << 2
};

constexpr auto is_bitmask_enum(BufferFlags) -> bool { return true; }


struct alignas(8) Buffer : public ResourceManager::Resource<BufferId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   BufferFlags flags_,
					   uint64_t sizeInBytes_,
					   uint8_t* data_ = nullptr) -> BufferHandle;
	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   BufferFlags flags_,
					   std::vector<float> const& floats_) -> BufferHandle;

	constexpr auto canBeDMASrc() const -> bool { return testUsageFlag(Usage::DMASrc); }

	constexpr auto canBeDMADst() const { return testUsageFlag(Usage::DMADst); }

	constexpr auto canBeReadByShader() const { return testUsageFlag(Usage::ShaderRead); }

	constexpr auto canBeWrittenByShader() const { return testUsageFlag(Usage::ShaderWrite); }

	constexpr auto canBeReadByVertex() const -> bool { return testUsageFlag(Usage::VertexRead); }

	constexpr auto canBeReadByIndex() const { return testUsageFlag(Usage::IndexRead); }

	constexpr auto canBeReadByIndirect() const { return testUsageFlag(Usage::IndirectRead); }

	constexpr auto canBeReadByTexture() const { return testUsageFlag(Usage::TextureRead); }

	constexpr auto canBeWrittenAsTexture() const { return testUsageFlag(Usage::TextureWrite); }

	auto getData() const -> uint8_t* { return (uint8_t*) (this + 1); }
	uint64_t sizeInBytes;
	BufferFlags flags;
	uint32_t padd;

	constexpr auto testUsageFlag(Usage flag_) const -> bool
	{
		return Core::bitmask::test_equal(ToUsage(flags), flag_);
	}

	static constexpr auto ToUsage(BufferFlags flags_) -> Usage
	{
		using namespace Core::bitmask;
		return from_uint<Usage>(to_uint(flags_ & BufferFlags::Usage) >> 5);
	}

	static constexpr auto FromUsage(Usage usage) -> BufferFlags
	{
		using namespace Core::bitmask;
		return from_uint<BufferFlags>(to_uint(usage) << 5);
	}
};

}

#endif //WYRD_RENDER_BUFFER_H
