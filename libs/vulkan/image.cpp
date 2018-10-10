

#include "core/core.h"
#include "vulkan/image.h"

namespace Vulkan
{

auto Image::pixelAt( 	unsigned int x_, unsigned int y_,
			  			unsigned int z_, unsigned int slice_ ) const -> Pixel
{
/*	case GenericTextureFormat::A2R10G10B10_SNORM_PACK32:
		if(channel_ == Channel::A)
			return fetchChannel_A2R10G10B10_UNORM( swizzle( ARGB, channel_ ), ptr_ );
		else
			return fetchChannel_A2R10G10B10_SNORM( swizzle( ARGB, channel_ ), ptr_ );
*/
}

}