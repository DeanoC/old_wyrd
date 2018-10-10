#include "core/core.h"
#include "binny/writehelper.h"
#include "resourcemanager/resourceman.h"
#include "render/resourcehandlers.h"
#include "render/image.h"
#include "fmt/format.h"

namespace Render {

auto RegisterResourceHandlers( ResourceManager::ResourceMan& rm_ ) -> void
{
	using namespace ResourceManager;

	// register GenericImage handler
	static_assert(offsetof( GenericImage, dataStore ) % sizeof( uintptr_t ) == 0 );
	rm_.registerResourceHandler(
			GenericImage::Id,
			{0,
			 []( uint16_t majorVersion_, uint16_t minorVersion_, int stage,
				 std::shared_ptr<void> ptr_ ) -> bool
			 {
				 if(majorVersion_ != GenericImage::MajorVersion) return false;
				 assert( stage == 0 );

				 auto image = std::static_pointer_cast<GenericImage>( ptr_ );
				 if(image->width == 0) return false;
				 if(image->height == 0) return false;
				 if(image->depth == 0) return false;
				 if(image->slices == 0) return false;
				 if(image->dataSize != image->computeDataSize()) return false;

				 return true;
			 },
			 []( int, void * )
			 { return true; }
			},
			[]( ResourceBase::ConstPtr ptr )
			{ return false; },
			[]( ResourceBase::ConstPtr ptr_, ResourceManager::ISaver& saver_ )
			{
				using namespace std::string_literals;
				auto image = std::static_pointer_cast<GenericImage const>( ptr_ );
				saver_.setMajorVersion( GenericImage::MajorVersion );
				saver_.setMinorVersion( GenericImage::MinorVersion );
				saver_.setWriterFunction(
						[image]( Binny::WriteHelper& h )
						{
							h.allow_nan( false );
							h.allow_infinity( true );
							h.set_default_type<uint32_t>();
							h.set_address_length( 64 );
							h.write_as<uint64_t>( image->dataSize, "data size" );
							h.write_as<uint64_t>( image->subClassData, "Subclass Data" );
							h.write( image->width, "Width" );
							h.write( image->height, "Height" );
							h.write( image->depth, "Depth" );
							h.write( image->slices, "Array size" );
							h.write((uint32_t) image->format,
									fmt::format( "Format: {0}", GtfCracker::name( image->format )));
							h.align();
							h.write_byte_array( image->dataStore, image->dataSize );
						} );
				return true;
			}
	);


}

} // end namespace