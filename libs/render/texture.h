#pragma once
#ifndef WYRD_RENDER_TEXTURE_H
#define WYRD_RENDER_TEXTURE_H

#include "core/core.h"
#include "core/utils.h"
#include "resourcemanager/base.h"
#include "render/generictextureformat.h"
#include "render/image.h"

namespace ResourceManager{ class ResourceMan; }
namespace Render {

class Texture : public ResourceManager::Resource<"TXTR"_resource_id>
{
public:
	static auto RegisterResourceHandler( ResourceManager::ResourceMan& rm_) -> void;

	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	// should we keep a copy of the texture data after init?
	static constexpr uint32_t KeepCpuCopyFlag = Core::Bit(0); // TODO ignored currently
	// we want to data to be set to zero on init
	static constexpr uint32_t InitZeroFlag = Core::Bit(1);
	static constexpr uint32_t CubeMapFlag = Core::Bit(2);
	static constexpr uint32_t ComputeMipMapsFlag = Core::Bit(3);
	static constexpr uint32_t UsageTransferSrc = Core::Bit(4);
	static constexpr uint32_t UsageTransferDst = Core::Bit(5);
	static constexpr uint32_t UsageShaderRead = Core::Bit(6);
	static constexpr uint32_t UsageShaderWrite = Core::Bit(7);
	static constexpr uint32_t UsageRopRead = Core::Bit(8);
	static constexpr uint32_t UsageRopWrite = Core::Bit(9);

	constexpr auto is1D() const { return height == 1 && depth == 1; }
	constexpr auto is2D() const { return depth == 1; }
	constexpr auto is3D() const { return depth != 1; }
	constexpr auto isCubeMap() const { return flags & CubeMapFlag; };

	constexpr auto canBeTransferSrc() const { return flags & UsageTransferSrc; }
	constexpr auto canBeTransferDst() const { return flags & UsageTransferDst; }
	constexpr auto canBeShaderRead() const { return flags & UsageShaderRead; }
	constexpr auto canBeShaderWrite() const { return flags & UsageShaderWrite; }
	constexpr auto canBeRopRead() const { return flags & UsageRopRead; }
	constexpr auto canBeRopWrite() const { return flags & UsageRopWrite; }

	// if the ComputeMipMapsFlag flag is set, the mips aren't actually stored here
	// the withComputedMipMaps param will return the size as if the were
	// and false for the real size stored in this texture
	constexpr auto computeSize(bool withComputedMipMaps_) const -> size_t;

	uint32_t 				flags;					//!< flags for this texture
	uint32_t 				width;					//!< width of this texture
	uint32_t 				height;					//!< height of this texture
	uint32_t 				depth;					//!< 3D depth

	uint32_t 				slices;					//!< slice count of this texture
	uint32_t 				mipLevels;				//!< number of mip levels
	uint32_t 				samples;				//!< sample count (usually 1)
	GenericTextureFormat	format;					//!< format of this texture

	GenericImage::Handle	imageHandle;

};
}

#endif //WYRD_GPU_TEXTURE_H
