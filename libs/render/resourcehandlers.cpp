#include "core/core.h"
#include "binny/writehelper.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/writer.h"
#include "render/resourcehandlers.h"
#include "render/image.h"
#include "fmt/format.h"
#include "texture.h"

namespace Render {

auto RegisterResourceHandlers( ResourceManager::ResourceMan& rm_ ) -> void
{
	using namespace ResourceManager;

	// register GenericImage handler
	static_assert(offsetof( GenericImage, dataStore ) % sizeof( uintptr_t ) == 0 );
	rm_.registerResourceHandler(
			GenericImage::Id,
			{0,
			 []( int stage, ResourceManager::ResolverInterface resolver_, uint16_t majorVersion_, uint16_t minorVersion_,
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
						[image]( Writer& w_ )
						{
							w_.allow_nan( false );
							w_.allow_infinity( true );
							w_.set_default_type<uint32_t>();
							w_.set_address_length( 64 );
							w_.write_as<uint64_t>( image->dataSize, "data size" );
							w_.write_as<uint64_t>( image->subClassData, "Subclass Data" );
							w_.write( image->width, "Width" );
							w_.write( image->height, "Height" );
							w_.write( image->depth, "Depth" );
							w_.write( image->slices, "Array size" );
							w_.write((uint32_t) image->format,
									fmt::format( "Format: {0}", GtfCracker::name( image->format )));
							w_.align();
							w_.write_byte_array( image->dataStore, image->dataSize );
						} );
				return true;
			}
	);

	Texture::RegisterResourceHandler(rm_);

}

} // end namespace