#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "binny/writehelper.h"

namespace ResourceManager {
auto TextResource::RegisterResourceHandler( ResourceManager::ResourceMan& rm_ ) -> void
{
	rm_.registerResourceHandler( TextResource::Id,
		{ 0,
			[]( int stage_, uint16_t majorVersion_, uint16_t minorVersion_, ResourceBase::Ptr ptr_ ) -> bool
			{
				if(majorVersion_ != 0) return false;
				if(minorVersion_ != 0) return false;
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
					[textResource](Binny::WriteHelper& h)
					{
						h.add_string(textResource->getText());
					});
			return true;
		}
	);
}

}