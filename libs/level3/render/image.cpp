#include "core/core.h"
#include "fmt/format.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/resourcename.h"
#include "resourcemanager/writer.h"
#include "binny/writehelper.h"
#include "render/image.h"

namespace Render {
auto GenericImage::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_,
				   std::shared_ptr<void> ptr_) -> bool
	{
		if(majorVersion_ != GenericImage::MajorVersion) return false;
		assert(stage == 0);

		auto image = std::static_pointer_cast<GenericImage>(ptr_);
		if(image->width == 0) return false;
		if(image->height == 0) return false;
		if(image->depth == 0) return false;
		if(image->slices == 0) return false;
		if(image->dataSize != image->computeDataSize()) return false;

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
	auto save = [](std::shared_ptr<ResourceBase const> ptr_, ResourceManager::ISaver& saver_)
	{
		using namespace std::string_literals;
		auto image = std::static_pointer_cast<GenericImage const>(ptr_);
		saver_.setMajorVersion(GenericImage::MajorVersion);
		saver_.setMinorVersion(GenericImage::MinorVersion);
		saver_.setWriterFunction(
				[image](ResourceManager::Writer& w_)
				{
					w_.allow_nan(false);
					w_.allow_infinity(true);
					w_.set_default_type<uint32_t>();
					w_.set_address_length(64);
					w_.write_as<uint64_t>(image->dataSize, "data size");
					w_.write_as<uint64_t>(image->subClassData, "Subclass Data");
					w_.write(image->width, "Width");
					w_.write(image->height, "Height");
					w_.write(image->depth, "Depth");
					w_.write(image->slices, "Array size");
					w_.write((uint32_t) image->format,
							 fmt::format("Format: {0}", GtfCracker::name(image->format)));
					w_.align();
					w_.write_byte_array(image->data(), image->dataSize);
				});
		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);

}

auto GenericImage::Create(
		ResourceManager::ResourceMan::Ptr rm_,
		ResourceManager::ResourceNameView const& name_,
		uint32_t width_,
		uint32_t height_,
		uint32_t depth_,
		uint32_t slices_,
		GenericTextureFormat fmt_) -> GenericImageHandle
{
	size_t const dataSize = computeDataSize(width_, height_, depth_, slices_, fmt_);
	size_t const totalSize = Core::alignTo(sizeof(Image) + dataSize, 8);
	auto* obj = (GenericImage*) malloc(totalSize);
	std::memset(obj, 0, totalSize);

	uint8_t* dataPtr = ((uint8_t*) (obj + 1));
	obj->sizeAndStageCount = totalSize;
	obj->dataSize = dataSize;
	obj->width = width_;
	obj->height = height_;
	obj->depth = depth_;
	obj->slices = slices_;
	obj->format = fmt_;

	rm_->placeInStorage(name_, *obj);
	free(obj);

	return rm_->openByName<Id>(name_);
}

auto GenericImage::Create(
		ResourceManager::ResourceMan::Ptr rm_,
		ResourceManager::ResourceNameView const& name_,
		uint32_t width_,
		uint32_t height_,
		uint32_t depth_,
		uint32_t slices_,
		GenericTextureFormat fmt_,
		void const* data_) -> GenericImageHandle
{
	size_t const dataSize = computeDataSize(width_, height_, depth_, slices_, fmt_);
	size_t const totalSize = Core::alignTo(sizeof(Image) + dataSize, 8);
	auto* obj = (GenericImage*) malloc(totalSize);
	std::memset(obj, 0, sizeof(Image));

	uint8_t* dataPtr = ((uint8_t*) (obj + 1));
	obj->sizeAndStageCount = totalSize;
	obj->dataSize = dataSize;
	obj->width = width_;
	obj->height = height_;
	obj->depth = depth_;
	obj->slices = slices_;
	obj->format = fmt_;
	std::memcpy(dataPtr, data_, dataSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);

	return rm_->openByName<Id>(name_);
}

} // end namespace