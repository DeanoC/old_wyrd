
template<ResourceManager::ResourceId id_>
template<typename type_>
auto Image<id_>::fetchRaw(uint8_t const *ptr_) -> type_
{
	return *((type_ const *) ptr_);
}

template<ResourceManager::ResourceId id_>
template<typename type_>
auto Image<id_>::fetchHomoChannel(uint8_t channel_, uint8_t const *ptr_) -> double
{
	return (double)fetchRaw<type_>(ptr_ + (sizeof(type_) * channel_));
}

template<ResourceManager::ResourceId id_>
template<typename type_>
auto Image<id_>::fetchHomoChannel_NORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	return fetchHomoChannel<type_>(channel_, ptr_) / (double) std::numeric_limits<type_>::max();
}

template<ResourceManager::ResourceId id_>
template<typename type_>
auto Image<id_>::fetchHomoChannel_sRGB(uint8_t channel_, uint8_t const *ptr_) -> double
{
	return Math::sRGB2LinearRGB_channel(fetchHomoChannel<type_>(channel_, ptr_));
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchHomoChannel_nibble(uint8_t channel_, uint8_t const *ptr_) -> double
{
	uint8_t bite = *(ptr_ + ((int) channel_ / 2));
	return (double) (channel_ & 0x1) ?
					   ((bite >> 0) & 0xF) :
					   ((bite >> 4) & 0xF);
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchHomoChannel_nibble_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	return fetchHomoChannel_nibble(channel_, ptr_) / 15.0;
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_R5G6B5_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	uint16_t pixel = fetchHomoChannel<uint16_t>(0, ptr_);
	if(channel_ == 0) return ((double) ((pixel >> 11) & 0x1F)) / 31.0;
	else if(channel_ == 1) return ((double) ((pixel >> 5) & 0x3F)) / 63.0;
	else if(channel_ == 2) return ((double) ((pixel >> 0) & 0x1F)) / 31.0;
	else
	{
		assert((int) channel_ < 4);
		return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_R5G5B5A1_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	uint16_t pixel = fetchHomoChannel<uint16_t>(0, ptr_);
	uint32_t x = 0;
	if(channel_ == 0) x = (pixel >> 11) & 0x1F;
	else if(channel_ == 1) x = (pixel >> 6) & 0x1F;
	else if(channel_ == 2) x = (pixel >> 1) & 0x1F;
	else if(channel_ == 3) return ((double) ((pixel >> 0) & 0x1));
	else
	{ assert((int) channel_ < 4); }
	return ((double) (x)) / 31.0;
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_A1R5G5B5_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	uint16_t pixel = fetchHomoChannel<uint16_t>(0, ptr_);

	uint32_t x = 0;
	if(channel_ == 0) return ((double) ((pixel >> 15) & 0x1));
	else if(channel_ == 1) x = (pixel >> 10) & 0x1F;
	else if(channel_ == 2) x = (pixel >> 5) & 0x1F;
	else if(channel_ == 3) x = (pixel >> 0) & 0x1F;
	else
	{ assert((int) channel_ < 4); }
	return ((double) (x)) / 31.0;
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchHomoChannel_FP16(uint8_t channel_, uint8_t const *ptr_) -> double
{
	return (double) Math::half2float(fetchHomoChannel<uint16_t>(channel_, ptr_));
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_A2R10G10B10(uint8_t channel_, uint8_t const *ptr_) -> double
{
	// fake fetch a single 32 bit format
	uint32_t pixel = fetchHomoChannel<uint32_t>(0, ptr_);

	uint32_t x = 0;
	if(channel_ == 0) return ((double) ((pixel >> 30) & 0x3));
	else if(channel_ == 1) x = (pixel >> 20) & 0x3FF;
	else if(channel_ == 2) x = (pixel >> 10) & 0x3FF;
	else if(channel_ == 3) x = (pixel >> 0) & 0x3FF;
	else
	{ assert((int) channel_ < 4); }
	return ((double) (x));
}

/*
// alpha is unsigned on d3d, signed on Vulkan and format doesn't exist on Metal
template<bool isD3D = false>
static auto fetchChannel_A2R10G10B10_signed( uint8_t channel_, uint8_t const *ptr_ ) -> double
{
	// fake fetch a single 32 bit format
	uint32_t pixel = fetchHomoChannel<uint32_t>( 0, ptr_ );

	static const uint16_t SignExtend[] = {0, 0xFC00};
	uint32_t x = 0;
	if(channel_ == 0)
	{
		static const uint8_t SignExtendA2[] = {0, 0xFC};
		uint8_t aval = ((pixel & 0xC0000000) >> 30);
		return (isD3D ? aval : (int8_t)SignExtendA2[x>>1]);
	}
	else if(channel_ == 1) x = (pixel >> 20) & 0x3FF;
	else if(channel_ == 2) x = (pixel >> 10) & 0x3FF;
	else if(channel_ == 3) x = (pixel >> 0) & 0x3FF;
	else
	{ assert((int) channel_ < 4 ); }
	return ((double) (int16_t) (x | SignExtend[x >> 9]));
}
 template<uint32_t id_>
auto Image<id_>::fetchChannel_A2R10G10B10_SNORM( uint8_t channel_, uint8_t const *ptr_ ) -> double
{
	// alpha is unsigned?
	if(channel_ == 0)
		return fetchChannel_A2R10G10B10_signed( 0, ptr_ ) / 3.0;
	else
		return fetchChannel_A2R10G10B10_signed( channel_, ptr_ ) / 511.0;
}
*/

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_A2R10G10B10_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	if(channel_ == 0)
		return fetchChannel_A2R10G10B10(0, ptr_) / 3.0;
	else
		return fetchChannel_A2R10G10B10(channel_, ptr_) / 1023.0;
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_X8D24_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	// fake fetch a single 32 bit format
	uint32_t pixel = fetchHomoChannel<uint32_t>(0, ptr_);
	if(channel_ == 0) return ((double) ((pixel & 0xFF000000) >> 24) / 255.0);
	else return ((double) (pixel & 0x00FFFFFF) / 16777215.0);
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_D24X8_UNORM(uint8_t channel_, uint8_t const *ptr_) -> double
{
	uint32_t pixel = fetchHomoChannel<uint32_t>(0, ptr_);
	if(channel_ == 0) return ((double) (pixel & 0x000000FF) / 255.0);
	else return ((double) ((pixel & 0xFFFFFF00) >> 8) / 16777215.0);
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::fetchChannel_D16S8_UNORM_UINT(uint8_t channel_, uint8_t const *ptr_) -> double
{
	if(channel_ == 0)
		return fetchHomoChannel_NORM<uint16_t>(channel_, ptr_);
	else
		return fetchHomoChannel<uint8_t>(1, ptr_);
}


template<ResourceManager::ResourceId id_>
auto Image<id_>::compressedChannelAt(Channel channel_, size_t index_) const -> double
{
	assert(false);
	return 0.0;
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth256ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R64G64B64A64_UINT:
			return fetchHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64G64B64A64_SINT:
			return fetchHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64G64B64A64_SFLOAT:
			return fetchHomoChannel<double>(swizzle(fmt_, channel_), ptr_);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth256ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth192ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R64G64B64_UINT:
			return fetchHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64G64B64_SINT:
			return fetchHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64G64B64_SFLOAT:
			return fetchHomoChannel<double>(swizzle(fmt_, channel_), ptr_);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth192ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth128ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{

	switch(fmt_)
	{
		case GenericTextureFormat::R64G64_UINT:
			return fetchHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64G64_SINT:
			return fetchHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64G64_SFLOAT:
			return fetchHomoChannel<double>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32B32A32_UINT:
			return fetchHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32B32A32_SINT:
			return fetchHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32B32A32_SFLOAT:
			return fetchHomoChannel<float>(swizzle(fmt_, channel_), ptr_);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth128ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth96ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R32G32B32_UINT:
			return fetchHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32B32_SINT:
			return fetchHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32B32_SFLOAT:
			return fetchHomoChannel<float>(swizzle(fmt_, channel_), ptr_);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth96ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth64ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R64_UINT:
			return fetchHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64_SINT:
			return fetchHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_); // potentially lossy!
		case GenericTextureFormat::R64_SFLOAT:
			return fetchHomoChannel<double>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32_UINT:
			return fetchHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32_SINT:
			return fetchHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32G32_SFLOAT:
			return fetchHomoChannel<float>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16A16_UNORM:
			return fetchHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16A16_SNORM:
			return fetchHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16A16_UINT:
		case GenericTextureFormat::R16G16B16A16_USCALED:
			return fetchHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16A16_SINT:
		case GenericTextureFormat::R16G16B16A16_SSCALED:
			return fetchHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16A16_SFLOAT:
			return fetchHomoChannel_FP16(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::D32_SFLOAT_S8_UINT:
			if(channel_ == Channel::R)
				return fetchHomoChannel<float>(0, ptr_);
			else
				return fetchHomoChannel<uint8_t>(0, ptr_ + 4);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth64ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth48ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R16G16B16_UNORM:
			return fetchHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16_SNORM:
			return fetchHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16_UINT:
		case GenericTextureFormat::R16G16B16_USCALED:
			return fetchHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16_SINT:
		case GenericTextureFormat::R16G16B16_SSCALED:
			return fetchHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16B16_SFLOAT:
			return fetchHomoChannel_FP16(swizzle(fmt_, channel_), ptr_);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth48ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth32ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R32_UINT:
			return fetchHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R32_SINT:
			return fetchHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::D32_SFLOAT:
		case GenericTextureFormat::R32_SFLOAT:
			return fetchHomoChannel<float>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16_UNORM:
			return fetchHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16_SNORM:
			return fetchHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16_UINT:
		case GenericTextureFormat::R16G16_USCALED:
			return fetchHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16_SINT:
		case GenericTextureFormat::R16G16_SSCALED:
			return fetchHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16G16_SFLOAT:
			return fetchHomoChannel_FP16(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8A8_UNORM:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8A8_SNORM:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8A8_USCALED:
		case GenericTextureFormat::R8G8B8A8_UINT:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8A8_SSCALED:
		case GenericTextureFormat::R8G8B8A8_SINT:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8A8_SRGB:
			if(channel_ == Channel::A)
				return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
			else
				return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8A8_UNORM:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8A8_SNORM:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8A8_USCALED:
		case GenericTextureFormat::B8G8R8A8_UINT:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8A8_SSCALED:
		case GenericTextureFormat::B8G8R8A8_SINT:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8A8_SRGB:
			if(channel_ == Channel::A)
				return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
			else
				return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
		case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
		case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			if(channel_ == Channel::A)
				return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
			else
				return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			return fetchChannel_A2R10G10B10_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
		case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			return fetchChannel_A2R10G10B10(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			return fetchChannel_A2R10G10B10_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
		case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
			return fetchChannel_A2R10G10B10(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::X8_D24_UNORM_PACK32:
			return fetchChannel_X8D24_UNORM((int) channel_, ptr_);
		case GenericTextureFormat::D24_UNORM_S8_UINT:
			return fetchChannel_D24X8_UNORM((int) channel_, ptr_);

			// TODO decoders for these exotic formats
//		case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//		case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth32ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth24ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R8G8B8_UNORM:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8_SNORM:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8_USCALED:
		case GenericTextureFormat::R8G8B8_UINT:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8_SSCALED:
		case GenericTextureFormat::R8G8B8_SINT:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8B8_SRGB:
			return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);

		case GenericTextureFormat::B8G8R8_UNORM:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8_SNORM:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8_USCALED:
		case GenericTextureFormat::B8G8R8_UINT:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8_SSCALED:
		case GenericTextureFormat::B8G8R8_SINT:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B8G8R8_SRGB:
			return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);

		case GenericTextureFormat::D16_UNORM_S8_UINT:
			return fetchChannel_D16S8_UNORM_UINT((int) channel_, ptr_);

		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth24ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth16ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			return fetchHomoChannel_nibble_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			return fetchHomoChannel_nibble_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			return fetchChannel_R5G6B5_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			return fetchChannel_R5G6B5_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			return fetchChannel_R5G5B5A1_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
			return fetchChannel_R5G5B5A1_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
			return fetchChannel_A1R5G5B5_UNORM(swizzle(fmt_, channel_), ptr_);

		case GenericTextureFormat::R8G8_UNORM:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8_SNORM:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8_USCALED:
		case GenericTextureFormat::R8G8_UINT:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8_SSCALED:
		case GenericTextureFormat::R8G8_SINT:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8G8_SRGB:
			return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::D16_UNORM:
		case GenericTextureFormat::R16_UNORM:
			return fetchHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16_SNORM:
			return fetchHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16_UINT:
		case GenericTextureFormat::R16_USCALED:
			return fetchHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16_SINT:
		case GenericTextureFormat::R16_SSCALED:
			return fetchHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R16_SFLOAT:
			return fetchHomoChannel_FP16(swizzle(fmt_, channel_), ptr_);
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth16ChannelAt";
			return 0.0;
	}
}

template<ResourceManager::ResourceId id_>
auto Image<id_>::bitWidth8ChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t const *ptr_) -> double
{
	switch(fmt_)
	{
		case GenericTextureFormat::R4G4_UNORM_PACK8:
			return fetchHomoChannel_nibble_UNORM(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8_UNORM:
			return fetchHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8_SNORM:
			return fetchHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::S8_UINT:
		case GenericTextureFormat::R8_USCALED:
		case GenericTextureFormat::R8_UINT:
			return fetchHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8_SSCALED:
		case GenericTextureFormat::R8_SINT:
			return fetchHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_);
		case GenericTextureFormat::R8_SRGB:
			return fetchHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_);

		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth8ChannelAt";
			return 0.0;
	}
}
