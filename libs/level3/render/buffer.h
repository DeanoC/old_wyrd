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
struct Encoder;

enum class BufferFlags : uint32_t
{
	NoInit = Core::Bit(0),
	InitZero = Core::Bit(1),
	CPUDynamic = Core::Bit(2), // not needed for irregular updated but faster for regular
	Usage = UsageMask << 3
};

constexpr auto is_bitmask_enum(BufferFlags) -> bool { return true; }


// resource stages of Buffer have to implement IGpuBuffer
struct IGpuBuffer
{
	virtual ~IGpuBuffer() = default;
	// update will allocate a host side buffer, copy the data in and then upload
	// via the devices DMA queue. Note not always performant but simple
	virtual auto update(uint8_t const* data_, uint64_t size_) -> void = 0;

	// requires CPUDynamic, provides cpu accessible pointer
	virtual auto map() -> void* = 0;
	virtual auto unmap() -> void = 0;
};

struct alignas(8) Buffer : public ResourceManager::Resource<BufferId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   BufferFlags flags_,
					   uint64_t sizeInBytes_,
					   void* data_ = nullptr) -> BufferHandle;

	// helper for uniform data buffers
	template<typename T>
	static auto Create(std::shared_ptr<ResourceManager::ResourceMan> rm_,
					   ResourceManager::ResourceNameView const& name_,
					   BufferFlags flags_,
					   std::vector<T> const& vals_) -> BufferHandle
	{
		return Create(rm_, name_, flags_, vals_.size() * sizeof(T), (uint8_t*) vals_.data());
	}

	constexpr auto canBeDMASrc() const -> bool { return testUsageFlag(Usage::DMASrc); }

	constexpr auto canBeDMADst() const { return testUsageFlag(Usage::DMADst); }

	constexpr auto canBeReadByShader() const { return testUsageFlag(Usage::ShaderRead); }

	constexpr auto canBeWrittenByShader() const { return testUsageFlag(Usage::ShaderWrite); }

	constexpr auto canBeReadByVertex() const -> bool { return testUsageFlag(Usage::VertexRead); }

	constexpr auto canBeReadByIndex() const { return testUsageFlag(Usage::IndexRead); }

	constexpr auto canBeReadByIndirect() const { return testUsageFlag(Usage::IndirectRead); }

	constexpr auto canBeReadByTexture() const { return testUsageFlag(Usage::TextureRead); }

	constexpr auto canBeWrittenAsTexture() const { return testUsageFlag(Usage::TextureWrite); }

	constexpr auto canBeUpdatedByCPU() const { return Core::bitmask::test_equal(flags, BufferFlags::CPUDynamic); }


	constexpr auto hasImplicitData() const -> bool {
		using namespace Core::bitmask;
		return !test_any(flags, BufferFlags::InitZero | BufferFlags::NoInit);
	}

	auto getData() const -> uint8_t const* { assert(hasImplicitData()); return (uint8_t const*) (this + 1); }
	auto getData() -> uint8_t* { assert(hasImplicitData()); return(uint8_t*) (this + 1); }

	// use if you've edited the implicit data (does update/map etc. for you)
	auto implicitDataChanged() const;

#define INTERFACE_THUNK(name) \
    template<typename... Args> auto name(Args... args) const -> void { \
        for(auto i = 0u; i < getStageCount(); ++i) \
        { \
        	auto iptr = getStage<IGpuBuffer>(i+1); \
            assert(iptr != nullptr); \
            iptr->name(args...); \
        }; \
	}

	INTERFACE_THUNK(update);
    INTERFACE_THUNK(unmap);

#undef INTERFACE_THUNK
    // map can only be done stage by stage so either pass the exact stage OR
    // ~0 (default) will use the first stage
	auto map(uint32_t stage_ = ~0) const -> void*
	{
		if(stage_ == ~0)
		{
			for(auto i = 0u; i < getStageCount(); ++i)
			{
				auto iptr = getStage<IGpuBuffer>(i+1);
				assert(iptr != nullptr);
				return iptr->map();
			};
			return nullptr;
		} else
		{
			auto iptr = getStage<IGpuBuffer>(stage_+1);
			assert(iptr != nullptr);
			return iptr->map();
		}
	}

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
