template<uint32_t id_>
template<typename type_>
auto Image<id_>::putHomoChannel(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	putRaw<type_>(ptr_ + (sizeof(type_) * channel_), (type_) value_);
}

template<uint32_t id_>
template<typename type_>
auto Image<id_>::putRaw(uint8_t *ptr_, type_ const value_) -> void
{
	type_ const v = Math::Clamp(value_, std::numeric_limits<type_>::min(), std::numeric_limits<type_>::max());

	*((type_ *) ptr_) = v;
}

template<uint32_t id_>
template<typename type_>
auto Image<id_>::putHomoChannel_NORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	putHomoChannel<type_>(channel_, ptr_, value_ * (double) std::numeric_limits<type_>::max());
}

template<uint32_t id_>
template<typename type_>
auto Image<id_>::putHomoChannel_sRGB(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	putHomoChannel<type_>(channel_, ptr_, Math::LinearRGB2sRGB_channel((float) value_));
}

template<uint32_t id_>
auto Image<id_>::putHomoChannel_nibble(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	// fetch the byte to merge the nibble into
	uint8_t bite = ptr_[channel_ / 2];
	uint8_t nibble = (uint8_t) Math::Clamp(value_, 0.0, 15.0);
	bite = (channel_ & uint8_t(0x1)) ?
		   	(bite & uint8_t(0xF0)) | (nibble << 0) :
			(bite & uint8_t(0x0F)) | (nibble << 4);
	ptr_[channel_ / 2] = bite;

}

template<uint32_t id_>
auto Image<id_>::putHomoChannel_nibble_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	putHomoChannel_nibble(channel_, ptr_, value_ * 15.0);
}

template<uint32_t id_>
auto Image<id_>::putChannel_R5G6B5_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	uint16_t pixel = fetchRaw<uint16_t>(ptr_);
	if(channel_ == 0)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0x07FF)) | ((uint16_t) v) << 11;
	} else if(channel_ == 1)
	{
		double const v = Math::Clamp(value_ * 63.0, 0.0, 63.0);
		pixel = (pixel & uint16_t(0xF81F)) | ((uint16_t) v) << 5;
	} else if(channel_ == 2)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0xFFE0)) | ((uint16_t) v) << 0;
	} else
	{
		assert(channel_ < 3);
	}
	putRaw(ptr_, pixel);
}

template<uint32_t id_>
auto Image<id_>::putChannel_R5G5B5A1_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	uint16_t pixel = fetchRaw<uint16_t>(ptr_);
	if(channel_ == 0)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0x07FF)) | ((uint16_t) v) << 11;
	} else if(channel_ == 1)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0xF83F)) | ((uint16_t) v) << 6;
	} else if(channel_ == 2)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0xFFC1)) | ((uint16_t) v) << 1;
	} else if(channel_ == 3)
	{
		double const v = Math::Clamp(value_, 0.0, 1.0);
		pixel = (pixel & uint16_t(0xFFFE)) | ((uint16_t) v) << 0;
	} else
	{
		assert(channel_ < 4);
	}
	putRaw(ptr_, pixel);
}

template<uint32_t id_>
auto Image<id_>::putChannel_A1R5G5B5_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	uint16_t pixel = fetchRaw<uint16_t>(ptr_);
	if(channel_ == 0)
	{
		double const v = Math::Clamp(value_, 0.0, 1.0);
		pixel = (pixel & uint16_t(0x7FFF)) | ((uint16_t) v) << 15;
	} else if(channel_ == 1)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0x83FF)) | ((uint16_t) v) << 10;
	} else if(channel_ == 2)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0xFC1F)) | ((uint16_t) v) << 5;
	} else if(channel_ == 3)
	{
		double const v = Math::Clamp(value_ * 31.0, 0.0, 31.0);
		pixel = (pixel & uint16_t(0xFFE0)) | ((uint16_t) v) << 0;
	} else
	{
		assert(channel_ < 4);
	}
	putRaw(ptr_, pixel);
}

template<uint32_t id_>
auto Image<id_>::putHomoChannel_FP16(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	// clamp half?
	uint16_t h = Math::float2half((float) value_);
	putRaw(ptr_ + (sizeof(uint16_t) * channel_), h);
}

template<uint32_t id_>
auto Image<id_>::putChannel_A2R10G10B10(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	uint32_t pixel = fetchRaw<uint32_t>(ptr_);
	if(channel_ == 0)
	{
		double const v = Math::Clamp(value_, 0.0, 3.0);
		pixel = (pixel & 0x3FFFFFFF) | ((uint32_t) v) << 30;
	} else if(channel_ == 1)
	{
		double const v = Math::Clamp(value_, 0.0, 1023.0);
		pixel = (pixel & 0xC00FFFFF) | ((uint32_t) v) << 20;
	} else if(channel_ == 2)
	{
		double const v = Math::Clamp(value_, 0.0, 1023.0);
		pixel = (pixel & 0xFFF003FF) | ((uint32_t) v) << 10;
	} else if(channel_ == 3)
	{
		double const v = Math::Clamp(value_, 0.0, 1023.0);
		pixel = (pixel & 0xFFFFFC00) | ((uint32_t) v) << 0;
	} else
	{
		assert(channel_ < 4);
	}
	putRaw(ptr_, pixel);

}

template<uint32_t id_>
auto Image<id_>::putChannel_A2R10G10B10_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	if(channel_ == 0) putChannel_A2R10G10B10(channel_, ptr_, value_ * 3.0);
	else putChannel_A2R10G10B10(channel_, ptr_, value_ * 1023.0);
}

template<uint32_t id_>
auto Image<id_>::putChannel_X8D24_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	uint32_t pixel = fetchRaw<uint32_t>(ptr_);
	if(channel_ == 0)
	{
		double const v = Math::Clamp(value_ * 255.0, 0.0, 255.0);
		pixel = (pixel & 0x00FFFFFF) | ((uint32_t) v) << 24;
	} else if(channel_ == 1)
	{
		static const double Max24Bit = double(1 << 24) - 1.0;
		double const v = Math::Clamp(value_ * Max24Bit, 0.0, Max24Bit);
		pixel = (pixel & 0xFF000000) | ((uint32_t) v) << 0;
	} else
	{
		assert(channel_ < 2);
	}
	putRaw(ptr_, pixel);
}

template<uint32_t id_>
auto Image<id_>::putChannel_D24X8_UNORM(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	uint32_t pixel = fetchRaw<uint32_t>(ptr_);
	if(channel_ == 0)
	{
		static const double Max24Bit = double(1 << 24) - 1.0;
		double const v = Math::Clamp(value_ * Max24Bit, 0.0, Max24Bit);
		pixel = (pixel & 0x000000FF) | ((uint32_t) v) << 8;
	} else if(channel_ == 1)
	{
		double const v = Math::Clamp(value_ * 255.0, 0.0, 255.0);
		pixel = (pixel & 0xFFFFFF00) | ((uint32_t) v) << 0;
	} else
	{
		assert(channel_ < 2);
	}
	putRaw(ptr_, pixel);
}

template<uint32_t id_>
auto Image<id_>::putChannel_D16S8_UNORM_UINT(uint8_t channel_, uint8_t *ptr_, double const value_) -> void
{
	if(channel_ == 0)
	{
		double const v = Math::Clamp(value_ * 65535.0, 0.0, 65535.0);
		putHomoChannel<uint16_t>(0, ptr_, (uint16_t) v);
	} else if(channel_ == 1)
	{
		double const v = Math::Clamp(value_, 0.0, 255.0);
		putHomoChannel<uint16_t>(0, ptr_ + 2, (uint8_t) v);
	} else
	{
		assert(channel_ < 2);
	}

}

template<uint32_t id_>
auto Image<id_>::bitWidth256SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										 double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R64G64B64A64_UINT:
			putHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R64G64B64A64_SINT:
			putHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R64G64B64A64_SFLOAT:
			putHomoChannel<double>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth256SetChannelAt";
	}
}

template<uint32_t id_>
auto Image<id_>::bitWidth192SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										 double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R64G64B64_UINT:
			putHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R64G64B64_SINT:
			putHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R64G64B64_SFLOAT:
			putHomoChannel<double>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth192SetChannelAt";
	}
}

template<uint32_t id_>
auto Image<id_>::bitWidth128SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										 double const value_) -> void
{

	switch(fmt_)
	{
		case GenericTextureFormat::R64G64_UINT:
			putHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R64G64_SINT:
			putHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R64G64_SFLOAT:
			putHomoChannel<double>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32B32A32_UINT:
			putHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32B32A32_SINT:
			putHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32B32A32_SFLOAT:
			putHomoChannel<float>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth128SetChannelAt";
	}
}

template<uint32_t id_>
auto Image<id_>::bitWidth96SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R32G32B32_UINT:
			putHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32B32_SINT:
			putHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32B32_SFLOAT:
			putHomoChannel<float>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth96SetChannelAt";
	}

}

template<uint32_t id_>
auto Image<id_>::bitWidth64SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R64_UINT:
			putHomoChannel<uint64_t>(swizzle(fmt_, channel_), ptr_, value_); // potentially lossy!
			break;
		case GenericTextureFormat::R64_SINT:
			putHomoChannel<int64_t>(swizzle(fmt_, channel_), ptr_, value_); // potentially lossy!
			break;
		case GenericTextureFormat::R64_SFLOAT:
			putHomoChannel<double>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32_UINT:
			putHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32_SINT:
			putHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32G32_SFLOAT:
			putHomoChannel<float>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16A16_UNORM:
			putHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16A16_SNORM:
			putHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16A16_UINT:
		case GenericTextureFormat::R16G16B16A16_USCALED:
			putHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16A16_SINT:
		case GenericTextureFormat::R16G16B16A16_SSCALED:
			putHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16A16_SFLOAT:
			putHomoChannel_FP16(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::D32_SFLOAT_S8_UINT:
			if(channel_ == Channel::R)
				putHomoChannel<float>(0, ptr_, value_);
			else
				putHomoChannel<uint8_t>(0, ptr_ + 4, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth64SetChannelAt";
	}

}

template<uint32_t id_>
auto Image<id_>::bitWidth48SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R16G16B16_UNORM:
			putHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16_SNORM:
			putHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16_UINT:
		case GenericTextureFormat::R16G16B16_USCALED:
			putHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16_SINT:
		case GenericTextureFormat::R16G16B16_SSCALED:
			putHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16B16_SFLOAT:
			putHomoChannel_FP16(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth48SetChannelAt";
	}
}

template<uint32_t id_>
auto Image<id_>::bitWidth32SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R32_UINT:
			putHomoChannel<uint32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R32_SINT:
			putHomoChannel<int32_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::D32_SFLOAT:
		case GenericTextureFormat::R32_SFLOAT:
			putHomoChannel<float>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16_UNORM:
			putHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16_SNORM:
			putHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16_UINT:
		case GenericTextureFormat::R16G16_USCALED:
			putHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16_SINT:
		case GenericTextureFormat::R16G16_SSCALED:
			putHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16G16_SFLOAT:
			putHomoChannel_FP16(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8A8_UNORM:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8A8_SNORM:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8A8_USCALED:
		case GenericTextureFormat::R8G8B8A8_UINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8A8_SSCALED:
		case GenericTextureFormat::R8G8B8A8_SINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8A8_SRGB:
			if(channel_ == Channel::A)
				putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			else
				putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8A8_UNORM:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8A8_SNORM:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8A8_USCALED:
		case GenericTextureFormat::B8G8R8A8_UINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8A8_SSCALED:
		case GenericTextureFormat::B8G8R8A8_SINT:
			putHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8A8_SRGB:
			if(channel_ == Channel::A)
				putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			else
				putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A8B8G8R8_UNORM_PACK32:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A8B8G8R8_SNORM_PACK32:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A8B8G8R8_USCALED_PACK32:
		case GenericTextureFormat::A8B8G8R8_UINT_PACK32:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A8B8G8R8_SSCALED_PACK32:
		case GenericTextureFormat::A8B8G8R8_SINT_PACK32:
			putHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A8B8G8R8_SRGB_PACK32:
			if(channel_ == Channel::A)
				putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			else
				putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A2R10G10B10_UNORM_PACK32:
			putChannel_A2R10G10B10_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A2R10G10B10_USCALED_PACK32:
		case GenericTextureFormat::A2R10G10B10_UINT_PACK32:
			putChannel_A2R10G10B10(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A2B10G10R10_UNORM_PACK32:
			putChannel_A2R10G10B10_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A2B10G10R10_USCALED_PACK32:
		case GenericTextureFormat::A2B10G10R10_UINT_PACK32:
			putChannel_A2R10G10B10(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::X8_D24_UNORM_PACK32:
			putChannel_X8D24_UNORM((int) channel_, ptr_, value_);
			break;
		case GenericTextureFormat::D24_UNORM_S8_UINT:
			putChannel_D24X8_UNORM((int) channel_, ptr_, value_);
			break;

			// TODO decoders for these exotic formats
//		case GenericTextureFormat::B10G11R11_UFLOAT_PACK32:
//		case GenericTextureFormat::E5B9G9R9_UFLOAT_PACK32:
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth32SetChannelAt";
	}
}

template<uint32_t id_>
auto Image<id_>::bitWidth24SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R8G8B8_UNORM:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8_SNORM:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8_USCALED:
		case GenericTextureFormat::R8G8B8_UINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8_SSCALED:
		case GenericTextureFormat::R8G8B8_SINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8B8_SRGB:
			putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;

		case GenericTextureFormat::B8G8R8_UNORM:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8_SNORM:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8_USCALED:
		case GenericTextureFormat::B8G8R8_UINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8_SSCALED:
		case GenericTextureFormat::B8G8R8_SINT:
			putHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B8G8R8_SRGB:
			putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;

		case GenericTextureFormat::D16_UNORM_S8_UINT:
			putChannel_D16S8_UNORM_UINT((int) channel_, ptr_, value_);
			break;

		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth24SetChannelAt";
	}

}

template<uint32_t id_>
auto Image<id_>::bitWidth16SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
										double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R4G4B4A4_UNORM_PACK16:
			putHomoChannel_nibble_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B4G4R4A4_UNORM_PACK16:
			putHomoChannel_nibble_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R5G6B5_UNORM_PACK16:
			putChannel_R5G6B5_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B5G6R5_UNORM_PACK16:
			putChannel_R5G6B5_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R5G5B5A1_UNORM_PACK16:
			putChannel_R5G5B5A1_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::B5G5R5A1_UNORM_PACK16:
			putChannel_R5G5B5A1_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::A1R5G5B5_UNORM_PACK16:
			putChannel_A1R5G5B5_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;

		case GenericTextureFormat::R8G8_UNORM:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8_SNORM:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8_USCALED:
		case GenericTextureFormat::R8G8_UINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8_SSCALED:
		case GenericTextureFormat::R8G8_SINT:
			putHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8G8_SRGB:
			putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::D16_UNORM:
		case GenericTextureFormat::R16_UNORM:
			putHomoChannel_NORM<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16_SNORM:
			putHomoChannel_NORM<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16_UINT:
		case GenericTextureFormat::R16_USCALED:
			putHomoChannel<uint16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16_SINT:
		case GenericTextureFormat::R16_SSCALED:
			putHomoChannel<int16_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R16_SFLOAT:
			putHomoChannel_FP16(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth16SetChannelAt";
	}

}

template<uint32_t id_>
auto Image<id_>::bitWidth8SetChannelAt(Channel channel_, GenericTextureFormat fmt_, uint8_t *ptr_,
									   double const value_) -> void
{
	switch(fmt_)
	{
		case GenericTextureFormat::R4G4_UNORM_PACK8:
			putHomoChannel_nibble_UNORM(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8_UNORM:
			putHomoChannel_NORM<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8_SNORM:
			putHomoChannel_NORM<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::S8_UINT:
		case GenericTextureFormat::R8_USCALED:
		case GenericTextureFormat::R8_UINT:
			putHomoChannel<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8_SSCALED:
		case GenericTextureFormat::R8_SINT:
			putHomoChannel<int8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		case GenericTextureFormat::R8_SRGB:
			putHomoChannel_sRGB<uint8_t>(swizzle(fmt_, channel_), ptr_, value_);
			break;
		default:
			LOG_S(ERROR) << GtfCracker::name(fmt_) << " not handled by bitWidth8SetChannelAt";
	}

}
