#pragma once
#ifndef WYRD_RENDER_TEXTURE_H
#define WYRD_RENDER_TEXTURE_H

#include "core/core.h"
#include "core/utils.h"
#include "render/resources.h"
#include "render/generictextureformat.h"
#include "render/image.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/resourcename.h"

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {
struct Encoder;

enum class TextureFlags : uint32_t
{
	// should we keep a copy of the texture data after init?
			KeepCpuCopy = Core::Bit(0),
	NoInit = Core::Bit(1),
	InitZero = Core::Bit(2),
	CubeMap = Core::Bit(3),
	ComputeMipMaps = Core::Bit(4),
	Usage = UsageMask << 5
};

constexpr auto is_bitmask_enum(TextureFlags) -> bool { return true; }


// resource stages of Texture have to implement IGpuTexture
struct IGpuTexture
{
	virtual ~IGpuTexture() = default;
	virtual auto transitionToRenderTarget(std::shared_ptr<Encoder> const& encoder_) -> void = 0;
	virtual auto transitionToDMADest(std::shared_ptr<Encoder> const& encoder_) -> void = 0;
	virtual auto transitionToShaderSrc(std::shared_ptr<Encoder> const& encoder_) -> void = 0;
	virtual auto transitionToDMASrc(std::shared_ptr<Encoder> const& encoder_) -> void = 0;
};

struct alignas(8) Texture : public ResourceManager::Resource<TextureId>
{
public:
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;
	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			TextureFlags flags_,
			uint32_t width_,
			uint32_t height_,
			uint32_t depth_,
			uint32_t slices_,
			uint32_t mipLevels_,
			uint32_t samples_,
			GenericTextureFormat format_,
			GenericImageHandle imageHandle_ = {}
	) -> TextureHandle;

	constexpr auto is1D() const { return height == 1 && depth == 1; }

	constexpr auto is2D() const { return depth == 1; }

	constexpr auto is3D() const { return depth != 1; }

	constexpr auto isCubeMap() -> bool const { return Core::bitmask::test_equal(flags, TextureFlags::CubeMap); };

	constexpr auto canBeDMASrc() const -> bool { return testUsageFlag(Usage::DMASrc); }

	constexpr auto canBeDMADst() const { return testUsageFlag(Usage::DMADst); }

	constexpr auto canBeReadByShader() const { return testUsageFlag(Usage::ShaderRead); }

	constexpr auto canBeWrittenByShader() const { return testUsageFlag(Usage::ShaderWrite); }

	constexpr auto canBeReadByRop() const { return testUsageFlag(Usage::RopRead); }

	constexpr auto canBeWrittenByRop() const { return testUsageFlag(Usage::RopWrite); }

	// if the ComputeMipMapsFlag flag is set, the mips aren't actually stored here
	// the withComputedMipMaps param will return the size as if the were
	// and false for the real size stored in this texture
	constexpr auto computeSize(bool withComputedMipMaps_) const -> size_t;

#define INTERFACE_THUNK(name) \
    template<typename... Args> auto name(Args... args) const { \
        for(auto i = 0u; i < getStageCount(); ++i) \
        { auto iptr = getStage<IGpuTexture>(i+1); \
            assert(iptr != nullptr); return iptr->name(args...);  } }

	INTERFACE_THUNK(transitionToRenderTarget);

	INTERFACE_THUNK(transitionToDMADest);

	INTERFACE_THUNK(transitionToShaderSrc);

	INTERFACE_THUNK(transitionToDMASrc);

#undef INTERFACE_THUNK

	TextureFlags flags;                        //!< flags for this texture
	uint32_t width;                            //!< width of this texture
	uint32_t height;                        //!< height of this texture
	uint32_t depth;                            //!< 3D depth

	uint32_t slices;                        //!< slice count of this texture
	uint32_t mipLevels;                        //!< number of mip levels
	uint32_t samples;                        //!< sample count (usually 1)
	GenericTextureFormat format;            //!< format of this texture

	GenericImageHandle imageHandle;

	constexpr auto testUsageFlag(Usage flag_) const -> bool
	{
		return Core::bitmask::test_equal(ToUsage(flags), flag_);
	}

	static constexpr auto ToUsage(TextureFlags flags_) -> Usage
	{
		using namespace Core::bitmask;
		return from_uint<Usage>(to_uint(flags_ & TextureFlags::Usage) >> 5);
	}

	static constexpr auto FromUsage(Usage usage) -> TextureFlags
	{
		using namespace Core::bitmask;
		return from_uint<TextureFlags>(to_uint(usage) << 5);
	}
protected:
	Texture() = default;

};

}

#endif //WYRD_GPU_TEXTURE_H
