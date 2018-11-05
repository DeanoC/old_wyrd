

template<uint32_t id_>
auto Image<id_>::CreateZeroed(uint32_t width_, uint32_t height_, uint32_t depth_, uint32_t slices_,
							  Render::GenericTextureFormat fmt_) -> Ptr
{
	uint64_t const dataSize = computeDataSize(width_, height_, depth_, slices_, fmt_);
	void *ptr = malloc(sizeof(Image) + dataSize);
	uint8_t *dataPtr = ((uint8_t *) ptr) + sizeof(Image);
	std::memset(dataPtr, 0, dataSize);
	new(ptr) Image{dataSize, width_, height_, depth_, slices_, fmt_, dataPtr};
	return Ptr((Image *) ptr, &free);
}

template<uint32_t id_>
constexpr auto
Image<id_>::calculateIndex(unsigned int x_, unsigned int y_, unsigned int z_, unsigned int slice_) const -> size_t
{
	assert(x_ < width);
	assert(y_ < height);
	assert(z_ < depth);
	assert(slice_ < slices);

	size_t const size1D = width;
	size_t const size2D = width * height;
	size_t const size3D = width * height * depth;
	size_t const index = (slice_ * size3D) + (z_ * size2D) + (y_ * size1D) + x_;

	return index;
}

template<uint32_t id_>
auto Image<id_>::channelAt(Channel channel_, unsigned int x_, unsigned int y_, unsigned int z_,
						   unsigned int slice_) const -> double
{
	assert((int) channel_ < GtfCracker::channelCount(format));
	auto index = calculateIndex(x_, y_, z_, slice_);

	// seperate out the block compressed format first
	if(GtfCracker::isCompressed(format))
	{
		return compressedChannelAt(channel_, index);
	}

	// split into bit width grouped formats
	assert(GtfCracker::bitWidth(format) >= 8);
	uint8_t *pixelPtr = data() + index * (GtfCracker::bitWidth(format) / 8);

	switch(GtfCracker::bitWidth(format))
	{
		case 256:
			return bitWidth256ChannelAt(channel_, format, pixelPtr);
		case 192:
			return bitWidth192ChannelAt(channel_, format, pixelPtr);
		case 128:
			return bitWidth128ChannelAt(channel_, format, pixelPtr);
		case 96:
			return bitWidth96ChannelAt(channel_, format, pixelPtr);
		case 64:
			return bitWidth64ChannelAt(channel_, format, pixelPtr);
		case 48:
			return bitWidth48ChannelAt(channel_, format, pixelPtr);
		case 32:
			return bitWidth32ChannelAt(channel_, format, pixelPtr);
		case 24:
			return bitWidth24ChannelAt(channel_, format, pixelPtr);
		case 16:
			return bitWidth16ChannelAt(channel_, format, pixelPtr);
		case 8:
			return bitWidth8ChannelAt(channel_, format, pixelPtr);
		default:
			LOG_F(ERROR, "Bitwidth of format not supported");
			return 0.0;
	}
}

template<uint32_t id_>
auto Image<id_>::setPixelAt(Pixel const& pixel_, unsigned int x_, unsigned int y_, unsigned int z_,
							unsigned int slice_) -> void
{

	// intentional fallthrough on this switch statement
	switch(GtfCracker::channelCount(format))
	{
		case 4:
			setChannelAt(pixel_.a, Channel::A, x_, y_, z_, slice_);
		case 3:
			setChannelAt(pixel_.b, Channel::B, x_, y_, z_, slice_);
		case 2:
			setChannelAt(pixel_.g, Channel::G, x_, y_, z_, slice_);
		case 1:
			setChannelAt(pixel_.r, Channel::R, x_, y_, z_, slice_);
			break;
		default:
			assert(GtfCracker::channelCount(format) <= 4);
			assert(GtfCracker::channelCount(format) > 0);
			break;
	}
}

template<uint32_t id_>
auto
Image<id_>::pixelAt(unsigned int x_, unsigned int const y_, unsigned int z_, unsigned int const slice_) const -> Pixel
{
	Pixel pixel{0, 0, 0, 0};

	// intentional fallthrough on this switch statement
	switch(GtfCracker::channelCount(format))
	{
		case 4:
			pixel.a = channelAt(Channel::A, x_, y_, z_, slice_);
		case 3:
			pixel.b = channelAt(Channel::B, x_, y_, z_, slice_);
		case 2:
			pixel.g = channelAt(Channel::G, x_, y_, z_, slice_);
		case 1:
			pixel.r = channelAt(Channel::R, x_, y_, z_, slice_);
			break;
		default:
			assert(GtfCracker::channelCount(format) <= 4);
			assert(GtfCracker::channelCount(format) > 0);
			break;
	}

	return pixel;
}

template<uint32_t id_>
auto Image<id_>::setChannelAt(double value_, Channel channel_, unsigned int x_, unsigned int y_, unsigned int z_,
							  unsigned int slice_) -> void
{
	assert((int) channel_ < GtfCracker::channelCount(format));

	auto index = calculateIndex(x_, y_, z_, slice_);

	// block compressed not handled ye
	assert(!GtfCracker::isCompressed(format));

	// split into bit width grouped formats
	assert(GtfCracker::bitWidth(format) >= 8);
	uint8_t *pixelPtr = data() + index * (GtfCracker::bitWidth(format) / 8);

	switch(GtfCracker::bitWidth(format))
	{
		case 256:
			bitWidth256SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 192:
			bitWidth192SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 128:
			bitWidth128SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 96:
			bitWidth96SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 64:
			bitWidth64SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 48:
			bitWidth48SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 32:
			bitWidth32SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 24:
			bitWidth24SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 16:
			bitWidth16SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		case 8:
			bitWidth8SetChannelAt(channel_, format, pixelPtr, value_);
			break;
		default:
			LOG_S(ERROR) << "Bitwidth " << GtfCracker::bitWidth(format) << " of format " << GtfCracker::name(format)
						 << " not supported";
	}
}
