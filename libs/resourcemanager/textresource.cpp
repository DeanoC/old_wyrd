#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "resourcemanager/writer.h"

namespace ResourceManager {
auto TextResource::RegisterResourceHandler( ResourceManager::ResourceMan& rm_ ) -> void
{
	rm_.registerResourceHandler( TextResource::Id,
		{ 0,
			[]( int stage_, ResourceManager::ResolverInterface,
					uint16_t majorVersion_, uint16_t minorVersion_, ResourceBase::Ptr ptr_ ) -> bool
			{
				if(majorVersion_ != MajorVersion) return false;
				if(stage_ != 0) return false;

				return true;
			},
			[]( int, void * ) -> bool { return true; },
		},
		[](ResourceBase::ConstPtr ptr_) { return false; },
		[](ResourceBase::ConstPtr ptr_, ISaver& saver_) {
			using namespace std::string_literals;
			auto textResource = std::static_pointer_cast<TextResource const>( ptr_ );
			saver_.setMajorVersion( MajorVersion );
			saver_.setMinorVersion( MinorVersion );
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
	txt->stage0 = totalSize;

	rm_->placeInStorage(name_, *txt);
	free(txt);

	return rm_->openResourceByName<Id>(name_);
}

}