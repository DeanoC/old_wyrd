#pragma once
#ifndef WYRD_RENDER_TEXTURE_H
#define WYRD_RENDER_TEXTURE_H

#include "core/core.h"
#include "core/utils.h"
#include "resourcemanager/base.h"
#include "render/generictextureformat.h"
#include "render/image.h"

namespace ResourceManager {
class ResourceMan;

struct MemStorage;
}
namespace Render {
struct Encoder;

// resource stages of Texture have to implement IGpuTexture
struct IGpuTexture
{
	virtual ~IGpuTexture() = default;
	virtual auto transitionToRenderTarget(std::shared_ptr<Encoder> const& encoder_) -> void = 0;
	virtual auto transitionFromRenderTarget(std::shared_ptr<Encoder> const& encoder_) -> void = 0;
};

struct Texture : public ResourceManager::Resource<"TXTR"_resource_id>
{
public:
	using Ptr = std::shared_ptr<Texture>;
	using ConstPtr = std::shared_ptr<Texture const>;
	using WeakPtr = std::weak_ptr<Texture>;
	using ConstWeakPtr = std::weak_ptr<Texture const>;

	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static auto RegisterToMemoryStorage(std::string const& name_,
										Texture const& texture_,
										std::shared_ptr<ResourceManager::MemStorage> const& memStorage_) -> bool;

	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	constexpr auto is1D() const { return height == 1 && depth == 1; }

	constexpr auto is2D() const { return depth == 1; }

	constexpr auto is3D() const { return depth != 1; }

	constexpr auto isCubeMap() -> bool const { return Core::bitmask::test_equal(flags, TextureFlag::CubeMap); };

	constexpr auto canBeDMASrc() const -> bool { return Core::bitmask::test_equal(extractUsage(flags), Usage::DMASrc); }

	constexpr auto canBeDMADst() const { return Core::bitmask::test_equal(extractUsage(flags), Usage::DMADst); }

	constexpr auto canBeShaderRead() const { return Core::bitmask::test_equal(extractUsage(flags), Usage::ShaderRead); }

	constexpr auto canBeShaderWrite() const
	{
		return Core::bitmask::test_equal(extractUsage(flags),
										 Usage::ShaderWrite);
	}

	constexpr auto canBeRopRead() const { return Core::bitmask::test_equal(extractUsage(flags), Usage::RopRead); }

	constexpr auto canBeRopWrite() const { return Core::bitmask::test_equal(extractUsage(flags), Usage::RopWrite); }

	// if the ComputeMipMapsFlag flag is set, the mips aren't actually stored here
	// the withComputedMipMaps param will return the size as if the were
	// and false for the real size stored in this texture
	constexpr auto computeSize(bool withComputedMipMaps_) const -> size_t;

#define INTERFACE_THUNK(name) \
    template<typename... Args> auto name(Args... args) { \
        for(auto i = 0u; i < getStageCount(); ++i) \
        { \
            assert(getStage<IGpuTexture>(i) != nullptr); \
            return getStage<IGpuTexture>(i)->name(args...); \
        } \
    }

	INTERFACE_THUNK(transitionToRenderTarget);

	INTERFACE_THUNK(transitionFromRenderTarget);

#undef INTERFACE_THUNK

	TextureFlag flags;                    //!< flags for this texture
	uint32_t width;                    //!< width of this texture
	uint32_t height;                    //!< height of this texture
	uint32_t depth;                    //!< 3D depth

	uint32_t slices;                    //!< slice count of this texture
	uint32_t mipLevels;                //!< number of mip levels
	uint32_t samples;                //!< sample count (usually 1)
	GenericTextureFormat format;                    //!< format of this texture

	GenericImage::Handle imageHandle;
};

}

#endif //WYRD_GPU_TEXTURE_H
