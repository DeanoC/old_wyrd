#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "resourcemanager/writer.h"

namespace ResourceManager {
auto TextResource::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	rm_.registerHandler(TextResource::Id,
						{0,
						 [](int stage_,
							ResourceManager::ResolverInterface,
							uint16_t majorVersion_,
							uint16_t minorVersion_,
							std::shared_ptr<ResourceBase> ptr_) -> bool
						 {
							 if(majorVersion_ != MajorVersion) return false;
							 if(stage_ != 0) return false;

							 return true;
						 },
						 [](int, void*) -> bool
						 { return true; },
						},
						[](std::shared_ptr<ResourceBase const> ptr_)
						{ return false; },
						[](std::shared_ptr<ResourceBase const> ptr_, ISaver& saver_)
						{
							using namespace std::string_literals;
							auto textResource = std::static_pointer_cast<TextResource const>(ptr_);
							saver_.setMajorVersion(MajorVersion);
							saver_.setMinorVersion(MinorVersion);
							saver_.setWriterFunction(
									[textResource](Writer& w_)
									{
										w_.add_string(textResource->getText());
									});
							return true;
						}
	);
}

auto TextResource::Create(
		ResourceManager::ResourceMan::Ptr rm_,
		ResourceManager::ResourceNameView const& name_,
		std::string_view text_) -> TextResourceHandle
{
	uint64_t const dataSize = text_.size();
	bool addZero = false;
	if(text_[dataSize - 1] != 0) addZero = true;

	size_t totalSize = Core::alignTo(sizeof(TextResource) + dataSize + addZero, 8);

	TextResource* txt = (TextResource*) malloc(totalSize);
	std::memset(txt, 0, totalSize);

	uint8_t* dataPtr = ((uint8_t*) txt) + sizeof(TextResource);
	std::memcpy(dataPtr, text_.data(), text_.size());

	if(addZero)dataPtr[text_.size()] = 0;
	txt->sizeAndStageCount = totalSize;

	rm_->placeInStorage(name_, *txt);
	free(txt);

	return rm_->openByName<Id>(name_);
}

auto TextResource::CreateFromFile(
		ResourceManager::ResourceMan::Ptr rm_,
		ResourceManager::ResourceNameView const& name_,
		std::string_view filename_) -> TextResourceHandle
{
	std::ifstream stream(static_cast<std::string>(filename_), std::ifstream::binary | std::ifstream::in);
	if(stream.bad()) return {};

	stream.seekg(0, stream.end);
	size_t const dataSize = stream.tellg();
	stream.seekg(0, stream.beg);
	size_t totalSize = Core::alignTo(sizeof(TextResource) + dataSize + 1, 8);
	TextResource* txt = (TextResource*) malloc(totalSize);
	std::memset(txt, 0, totalSize);
	char* text = ((char*) txt) + sizeof(ResourceBase);
	stream.read(text, dataSize);
	stream.close();

	txt->sizeAndStageCount = totalSize;
	rm_->placeInStorage(name_, *txt);
	free(txt);

	return rm_->openByName<Id>(name_);
}
}