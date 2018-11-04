#include <core/quick_hash.h>
#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/memstorage.h"
#include "binny/writehelper.h"
#include "render/texture.h"

namespace Render {
auto Texture::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_,
				   uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto texture = std::static_pointer_cast<Texture>(ptr_);
		auto[getRMFunc, resolverFunc] = resolver_;
		resolverFunc(texture->imageHandle.base);

		if(Core::bitmask::test_equal(texture->flags, TextureFlag::InitZero))
		{
			if(texture->imageHandle.isValid()) return false;
		} else
		{
			if(!texture->imageHandle.isValid() ||
					texture->imageHandle.acquire() == nullptr)
				return false;
		}

		if(texture->width == 0) return false;
		if(texture->height == 0) return false;
		if(texture->depth == 0) return false;
		if(texture->slices == 0) return false;
		if(texture->samples == 0) return false;
		if(texture->mipLevels == 0)
		{
			// computing with 0 mipLevels means all of the them
			if(Core::bitmask::test_equal(texture->flags, TextureFlag::ComputeMipMaps))
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
	};

	auto destroy = [](int, void*)
	{
		return true;
	};
	auto changed = [](ResourceBase::ConstPtr ptr_)
	{
		return false;
	};
	auto save = [](ResourceBase::ConstPtr ptr_, ResourceManager::ISaver& writer_)
	{
		auto texture = std::static_pointer_cast<Texture const>(ptr_);

		return true;
	};

	rm_.registerResourceHandler(Id, {0, load, destroy}, changed, save);

}

auto Texture::PlaceInStorage(
		ResourceManager::ResourceNameView name_,
		Texture const& texture_,
		std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> bool
{
	using namespace std::string_view_literals;

	auto storage = rm_->getStorageForPrefix(name_.getStorage());
	assert(storage);
	switch(Core::QuickHash(name_.getStorage()))
	{
		case Core::QuickHash("mem"sv):
		{
			auto memstorage = std::static_pointer_cast<ResourceManager::MemStorage>(storage);
			return memstorage->addMemory(
					std::string(name_.getName()),
					Texture::Id,
					Texture::MajorVersion,
					Texture::MinorVersion,
					&texture_, sizeof(Render::Texture));
			break;
		}
		default:
			LOG_S(ERROR) << "Unknown storage type for PlaceInStore";
			return {};
	}
}

constexpr auto Texture::computeSize(bool withComputedMipMaps_) const -> size_t
{
	size_t texelCount = width * height * depth * slices * samples;
	size_t size = texelCount * GtfCracker::bitWidth(format) / 8;

	// don't add mipmaps size if not wanted and computed at upload time
	if(Core::bitmask::test_equal(flags, TextureFlag::ComputeMipMaps) &&
	   withComputedMipMaps_ == false)
	{
		return size;
	}

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