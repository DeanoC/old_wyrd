#include "core/core.h"
#include "render/texture.h"
#include "render/gtfcracker.h"
#include "resourcemanager/resourceman.h"
#include "math/scalar_math.h"
#include "binny/writehelper.h"

namespace Render {
auto Texture::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	rm_.registerResourceHandler(Id,
		{0,
		 [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_, uint16_t minorVersion_,
			std::shared_ptr<void> ptr_) -> bool
		 {
			 assert(stage == 0);
			 if(majorVersion_ != MajorVersion) return false;
			 auto texture = std::static_pointer_cast<Texture>(ptr_);
			 auto [getRMFunc, resolverFunc] = resolver_;
			 resolverFunc(texture->imageHandle.base);

			 if(texture->flags & InitZeroFlag)
			 {
				 if(texture->imageHandle.acquire() != nullptr) return false;
			 } else
			 {
				 if(texture->imageHandle.acquire() == nullptr) return false;
			 }

			 if(texture->width == 0) return false;
			 if(texture->height == 0) return false;
			 if(texture->depth == 0) return false;
			 if(texture->slices == 0) return false;
			 if(texture->samples == 0) return false;
			 if(texture->mipLevels == 0)
			 {
				 // computing with 0 mipLevels means all of the them
				 if(texture->flags & ComputeMipMapsFlag)
				 {
					 texture->mipLevels = Math::log2(std::max(texture->depth,
															  std::max(texture->width,
																	   texture->height)));
				 } else
				 {
					 return false;
				 }
			 }
			 if(texture->isCubeMap() && (texture->slices % 6) != 0) return false;

			 return true;
		 }, [](int, void *)
		 {
			 return true;
		 }},
		[](ResourceBase::ConstPtr ptr_)
		{
			return false;
		},
		[](ResourceBase::ConstPtr ptr_, ResourceManager::ISaver& writer_)
		{
			auto texture = std::static_pointer_cast<Texture const>(ptr_);

			return true;
		});
}

constexpr auto Texture::computeSize(bool withComputedMipMaps_) const -> size_t
{
	size_t texelCount = width * height * depth * slices * samples;
	size_t size = texelCount * GtfCracker::bitWidth(format) / 8;

	// don't add mipmaps size if not wanted and computed at upload time
	if((flags & ComputeMipMapsFlag) && withComputedMipMaps_ == false)
		return size;

	// compute the mip map levels

	size_t totalSize = size;
	// could use a log2...
	for(unsigned int j = 1; j < mipLevels; ++j)
	{
		if(is1D())
		{
			totalSize += size >> j;
		} else if(is2D())
		{
			totalSize += size >> (j * j);
		} else if(is3D())
		{
			totalSize += size >> (j * j * j);
		}
	}

	return totalSize;
}

} // end namespace