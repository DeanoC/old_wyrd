// Copyright (c) 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "source/comp/bit_stream.h"

namespace spvtools {
namespace comp {
namespace {

// Converts |buffer| to a stream of '0' and '1'.
template<typename T>
std::string BufferToStream(const std::vector<T>& buffer)
{
	std::stringstream ss;
	for(auto it = buffer.begin(); it != buffer.end(); ++it)
	{
		std::string str = std::bitset<sizeof(T) * 8>(*it).to_string();
		// Strings generated by std::bitset::to_string are read right to left.
		// Reversing to left to right.
		std::reverse(str.begin(), str.end());
		ss << str;
	}
	return ss.str();
}

// Converts a left-to-right input string of '0' and '1' to a buffer of |T|
// words.
template<typename T>
std::vector<T> StreamToBuffer(std::string str)
{
	// The input string is left-to-right, the input argument of std::bitset needs
	// to right-to-left. Instead of reversing tokens, reverse the entire string
	// and iterate tokens from end to begin.
	std::reverse(str.begin(), str.end());
	const int word_size = static_cast<int>(sizeof(T) * 8);
	const int str_length = static_cast<int>(str.length());
	std::vector<T> buffer;
	buffer.reserve(NumBitsToNumWords<sizeof(T)>(str.length()));
	for(int index = str_length - word_size; index >= 0; index -= word_size)
	{
		buffer.push_back(static_cast<T>(
								 std::bitset<sizeof(T) * 8>(str, index, word_size).to_ullong()));
	}
	const size_t suffix_length = str.length() % word_size;
	if(suffix_length != 0)
	{
		buffer.push_back(static_cast<T>(
								 std::bitset<sizeof(T) * 8>(str, 0, suffix_length).to_ullong()));
	}
	return buffer;
}

// Adds '0' chars at the end of the string until the size is a multiple of N.
template<size_t N>
std::string PadToWord(std::string&& str)
{
	const size_t tail_length = str.size() % N;
	if(tail_length != 0) str += std::string(N - tail_length, '0');
	return std::move(str);
}

// Adds '0' chars at the end of the string until the size is a multiple of N.
template<size_t N>
std::string PadToWord(const std::string& str)
{
	return PadToWord<N>(std::string(str));
}

// Converts a left-to-right stream of bits to std::bitset.
template<size_t N>
std::bitset<N> StreamToBitset(std::string str)
{
	std::reverse(str.begin(), str.end());
	return std::bitset<N>(str);
}

// Converts a left-to-right stream of bits to uint64.
uint64_t StreamToBits(std::string str)
{
	std::reverse(str.begin(), str.end());
	return std::bitset<64>(str).to_ullong();
}

// A simple and inefficient implementatition of BitWriterInterface,
// using std::stringstream. Intended for tests only.
class BitWriterStringStream : public BitWriterInterface
{
public:
	void WriteBits(uint64_t bits, size_t num_bits) override
	{
		assert(num_bits <= 64);
		ss_ << BitsToStream(bits, num_bits);
	}

	size_t GetNumBits() const override { return ss_.str().size(); }

	std::vector<uint8_t> GetDataCopy() const override
	{
		return StreamToBuffer<uint8_t>(ss_.str());
	}

	std::string GetStreamRaw() const { return ss_.str(); }

private:
	std::stringstream ss_;
};

// A simple and inefficient implementatition of BitReaderInterface.
// Intended for tests only.
class BitReaderFromString : public BitReaderInterface
{
public:
	explicit BitReaderFromString(std::string&& str)
			: str_(std::move(str)), pos_(0) {}

	explicit BitReaderFromString(const std::vector<uint64_t>& buffer)
			: str_(BufferToStream(buffer)), pos_(0) {}

	explicit BitReaderFromString(const std::vector<uint8_t>& buffer)
			: str_(PadToWord<64>(BufferToStream(buffer))), pos_(0) {}

	size_t ReadBits(uint64_t* bits, size_t num_bits) override
	{
		if(ReachedEnd()) return 0;
		std::string sub = str_.substr(pos_, num_bits);
		*bits = StreamToBits(sub);
		pos_ += sub.length();
		return sub.length();
	}

	size_t GetNumReadBits() const override { return pos_; }

	bool ReachedEnd() const override { return pos_ >= str_.length(); }

private:
	std::string str_;
	size_t pos_;
};

TEST(NumBitsToNumWords, Word8
) {
EXPECT_EQ(0u, NumBitsToNumWords<8>(0));
EXPECT_EQ(1u, NumBitsToNumWords<8>(1));
EXPECT_EQ(1u, NumBitsToNumWords<8>(7));
EXPECT_EQ(1u, NumBitsToNumWords<8>(8));
EXPECT_EQ(2u, NumBitsToNumWords<8>(9));
EXPECT_EQ(2u, NumBitsToNumWords<8>(16));
EXPECT_EQ(3u, NumBitsToNumWords<8>(17));
EXPECT_EQ(3u, NumBitsToNumWords<8>(23));
EXPECT_EQ(3u, NumBitsToNumWords<8>(24));
EXPECT_EQ(4u, NumBitsToNumWords<8>(25));
}

TEST(NumBitsToNumWords, Word64
) {
EXPECT_EQ(0u, NumBitsToNumWords<64>(0));
EXPECT_EQ(1u, NumBitsToNumWords<64>(1));
EXPECT_EQ(1u, NumBitsToNumWords<64>(64));
EXPECT_EQ(2u, NumBitsToNumWords<64>(65));
EXPECT_EQ(2u, NumBitsToNumWords<64>(128));
EXPECT_EQ(3u, NumBitsToNumWords<64>(129));
}

TEST(ZigZagCoding, Encode0
) {
EXPECT_EQ(0u, EncodeZigZag(0, 0));
EXPECT_EQ(1u, EncodeZigZag(-1, 0));
EXPECT_EQ(2u, EncodeZigZag(1, 0));
EXPECT_EQ(3u, EncodeZigZag(-2, 0));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 1,
EncodeZigZag(std::numeric_limits<int64_t>::max(),
0));
EXPECT_EQ(std::numeric_limits<uint64_t>::max(),
		EncodeZigZag(std::numeric_limits<int64_t>::min(), 0)
);
}

TEST(ZigZagCoding, Decode0
) {
EXPECT_EQ(0, DecodeZigZag(0, 0));
EXPECT_EQ(-1, DecodeZigZag(1, 0));
EXPECT_EQ(1, DecodeZigZag(2, 0));
EXPECT_EQ(-2, DecodeZigZag(3, 0));
EXPECT_EQ(std::numeric_limits<int64_t>::min(),
		DecodeZigZag(std::numeric_limits<uint64_t>::max(), 0)
);
EXPECT_EQ(std::numeric_limits<int64_t>::max(),
		DecodeZigZag(std::numeric_limits<uint64_t>::max() - 1, 0)
);
}

TEST(ZigZagCoding, Encode1
) {
EXPECT_EQ(0u, EncodeZigZag(0, 1));
EXPECT_EQ(1u, EncodeZigZag(1, 1));
EXPECT_EQ(2u, EncodeZigZag(-1, 1));
EXPECT_EQ(3u, EncodeZigZag(-2, 1));
EXPECT_EQ(4u, EncodeZigZag(2, 1));
EXPECT_EQ(5u, EncodeZigZag(3, 1));
EXPECT_EQ(6u, EncodeZigZag(-3, 1));
EXPECT_EQ(7u, EncodeZigZag(-4, 1));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 2,
EncodeZigZag(std::numeric_limits<int64_t>::max(),
1));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 1,
EncodeZigZag (std::numeric_limits<int64_t>::min()
+ 1, 1));
EXPECT_EQ(std::numeric_limits<uint64_t>::max(),
		EncodeZigZag(std::numeric_limits<int64_t>::min(), 1)
);
}

TEST(ZigZagCoding, Decode1
) {
EXPECT_EQ(0, DecodeZigZag(0, 1));
EXPECT_EQ(1, DecodeZigZag(1, 1));
EXPECT_EQ(-1, DecodeZigZag(2, 1));
EXPECT_EQ(-2, DecodeZigZag(3, 1));
EXPECT_EQ(2, DecodeZigZag(4, 1));
EXPECT_EQ(3, DecodeZigZag(5, 1));
EXPECT_EQ(-3, DecodeZigZag(6, 1));
EXPECT_EQ(-4, DecodeZigZag(7, 1));
EXPECT_EQ(std::numeric_limits<int64_t>::min(),
		DecodeZigZag(std::numeric_limits<uint64_t>::max(), 1)
);
EXPECT_EQ (std::numeric_limits<int64_t>::min()
+ 1,
DecodeZigZag (std::numeric_limits<uint64_t>::max()
- 1, 1));
EXPECT_EQ(std::numeric_limits<int64_t>::max(),
		DecodeZigZag(std::numeric_limits<uint64_t>::max() - 2, 1)
);
}

TEST(ZigZagCoding, Encode2
) {
EXPECT_EQ(0u, EncodeZigZag(0, 2));
EXPECT_EQ(1u, EncodeZigZag(1, 2));
EXPECT_EQ(2u, EncodeZigZag(2, 2));
EXPECT_EQ(3u, EncodeZigZag(3, 2));
EXPECT_EQ(4u, EncodeZigZag(-1, 2));
EXPECT_EQ(5u, EncodeZigZag(-2, 2));
EXPECT_EQ(6u, EncodeZigZag(-3, 2));
EXPECT_EQ(7u, EncodeZigZag(-4, 2));
EXPECT_EQ(8u, EncodeZigZag(4, 2));
EXPECT_EQ(9u, EncodeZigZag(5, 2));
EXPECT_EQ(10u, EncodeZigZag(6, 2));
EXPECT_EQ(11u, EncodeZigZag(7, 2));
EXPECT_EQ(12u, EncodeZigZag(-5, 2));
EXPECT_EQ(13u, EncodeZigZag(-6, 2));
EXPECT_EQ(14u, EncodeZigZag(-7, 2));
EXPECT_EQ(15u, EncodeZigZag(-8, 2));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 4,
EncodeZigZag(std::numeric_limits<int64_t>::max(),
2));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 3,
EncodeZigZag (std::numeric_limits<int64_t>::min()
+ 3, 2));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 2,
EncodeZigZag (std::numeric_limits<int64_t>::min()
+ 2, 2));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 1,
EncodeZigZag (std::numeric_limits<int64_t>::min()
+ 1, 2));
EXPECT_EQ(std::numeric_limits<uint64_t>::max(),
		EncodeZigZag(std::numeric_limits<int64_t>::min(), 2)
);
}

TEST(ZigZagCoding, Decode2
) {
EXPECT_EQ(0, DecodeZigZag(0, 2));
EXPECT_EQ(1, DecodeZigZag(1, 2));
EXPECT_EQ(2, DecodeZigZag(2, 2));
EXPECT_EQ(3, DecodeZigZag(3, 2));
EXPECT_EQ(-1, DecodeZigZag(4, 2));
EXPECT_EQ(-2, DecodeZigZag(5, 2));
EXPECT_EQ(-3, DecodeZigZag(6, 2));
EXPECT_EQ(-4, DecodeZigZag(7, 2));
EXPECT_EQ(4, DecodeZigZag(8, 2));
EXPECT_EQ(5, DecodeZigZag(9, 2));
EXPECT_EQ(6, DecodeZigZag(10, 2));
EXPECT_EQ(7, DecodeZigZag(11, 2));
EXPECT_EQ(-5, DecodeZigZag(12, 2));
EXPECT_EQ(-6, DecodeZigZag(13, 2));
EXPECT_EQ(-7, DecodeZigZag(14, 2));
EXPECT_EQ(-8, DecodeZigZag(15, 2));
EXPECT_EQ(std::numeric_limits<int64_t>::min(),
		DecodeZigZag(std::numeric_limits<uint64_t>::max(), 2)
);
EXPECT_EQ (std::numeric_limits<int64_t>::min()
+ 1,
DecodeZigZag (std::numeric_limits<uint64_t>::max()
- 1, 2));
EXPECT_EQ (std::numeric_limits<int64_t>::min()
+ 2,
DecodeZigZag (std::numeric_limits<uint64_t>::max()
- 2, 2));
EXPECT_EQ (std::numeric_limits<int64_t>::min()
+ 3,
DecodeZigZag (std::numeric_limits<uint64_t>::max()
- 3, 2));
EXPECT_EQ(std::numeric_limits<int64_t>::max(),
		DecodeZigZag(std::numeric_limits<uint64_t>::max() - 4, 2)
);
}

TEST(ZigZagCoding, Encode63
) {
EXPECT_EQ(0u, EncodeZigZag(0, 63));

for(
int64_t i = 0;
i < 0xFFFFFFFF; i += 1234567) {
const int64_t positive_val = GetLowerBits(i * i * i + i * i, 63) | 1UL;
ASSERT_EQ(static_cast
<uint64_t>(positive_val),
		EncodeZigZag(positive_val, 63)
);
ASSERT_EQ((1ULL << 63) - 1 + positive_val, EncodeZigZag(-positive_val, 63));
}

EXPECT_EQ((1ULL << 63) - 1,
EncodeZigZag(std::numeric_limits<int64_t>::max(),
63));
EXPECT_EQ (std::numeric_limits<uint64_t>::max()
- 1,
EncodeZigZag (std::numeric_limits<int64_t>::min()
+ 1, 63));
EXPECT_EQ(std::numeric_limits<uint64_t>::max(),
		EncodeZigZag(std::numeric_limits<int64_t>::min(), 63)
);
}

TEST(BufToStream, UInt8_Empty
) {
const std::string expected_bits = "";
std::vector<uint8_t> buffer = StreamToBuffer<uint8_t>(expected_bits);
EXPECT_TRUE(buffer
.
empty()
);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(expected_bits, result_bits
);
}

TEST(BufToStream, UInt8_OneWord
) {
const std::string expected_bits = "00101100";
std::vector<uint8_t> buffer = StreamToBuffer<uint8_t>(expected_bits);
EXPECT_EQ(std::vector<uint8_t>({static_cast<uint8_t>(
										StreamToBitset<8>(expected_bits).to_ulong())}),
		buffer
);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(expected_bits, result_bits
);
}

TEST(BufToStream, UInt8_MultipleWords
) {
const std::string expected_bits =
		"00100010"
		"01101010"
		"01111101"
		"00100010";
std::vector<uint8_t> buffer = StreamToBuffer<uint8_t>(expected_bits);
EXPECT_EQ(std::vector<uint8_t>({
									   static_cast<uint8_t>(StreamToBitset<8>("00100010").to_ulong()),
									   static_cast<uint8_t>(StreamToBitset<8>("01101010").to_ulong()),
									   static_cast<uint8_t>(StreamToBitset<8>("01111101").to_ulong()),
									   static_cast<uint8_t>(StreamToBitset<8>("00100010").to_ulong()),
							   }),
		buffer
);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(expected_bits, result_bits
);
}

TEST(BufToStream, UInt64_Empty
) {
const std::string expected_bits = "";
std::vector<uint64_t> buffer = StreamToBuffer<uint64_t>(expected_bits);
EXPECT_TRUE(buffer
.
empty()
);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(expected_bits, result_bits
);
}

TEST(BufToStream, UInt64_OneWord
) {
const std::string expected_bits =
		"0001000111101110011001101010101000100010110011000100010010001000";
std::vector<uint64_t> buffer = StreamToBuffer<uint64_t>(expected_bits);
ASSERT_EQ(1u, buffer.
size()
);
EXPECT_EQ(0x1122334455667788u, buffer[0]);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(expected_bits, result_bits
);
}

TEST(BufToStream, UInt64_Unaligned
) {
const std::string expected_bits =
		"0010001001101010011111010010001001001010000111110010010010010101"
		"0010001001101010011111111111111111111111";
std::vector<uint64_t> buffer = StreamToBuffer<uint64_t>(expected_bits);
EXPECT_EQ(std::vector<uint64_t>({
										StreamToBits(expected_bits.substr(0, 64)),
										StreamToBits(expected_bits.substr(64, 64)),
								}),
		buffer
);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(PadToWord<64>(expected_bits), result_bits
);
}

TEST(BufToStream, UInt64_MultipleWords
) {
const std::string expected_bits =
		"0010001001101010011111010010001001001010000111110010010010010101"
		"0010001001101010011111111111111111111111000111110010010010010111"
		"0000000000000000000000000000000000000000000000000010010011111111";
std::vector<uint64_t> buffer = StreamToBuffer<uint64_t>(expected_bits);
EXPECT_EQ(std::vector<uint64_t>({
										StreamToBits(expected_bits.substr(0, 64)),
										StreamToBits(expected_bits.substr(64, 64)),
										StreamToBits(expected_bits.substr(128, 64)),
								}),
		buffer
);
const std::string result_bits = BufferToStream(buffer);
EXPECT_EQ(expected_bits, result_bits
);
}

TEST(PadToWord, Test
) {
EXPECT_EQ("10100000", PadToWord<8>("101"));
EXPECT_EQ(
"10100000"
"00000000",
PadToWord<16>("101"));
EXPECT_EQ(
"10100000"
"00000000"
"00000000"
"00000000",
PadToWord<32>("101"));
EXPECT_EQ(
"10100000"
"00000000"
"00000000"
"00000000"
"00000000"
"00000000"
"00000000"
"00000000",
PadToWord<64>("101"));
}

TEST(BitWriterStringStream, Empty
) {
BitWriterStringStream writer;
EXPECT_EQ(0u, writer.
GetNumBits()
);
EXPECT_EQ(0u, writer.
GetDataSizeBytes()
);
EXPECT_EQ("", writer.
GetStreamRaw()
);
}

TEST(BitWriterStringStream, WriteBits
) {
BitWriterStringStream writer;
const uint64_t bits1 = 0x1 | 0x2 | 0x10;
writer.
WriteBits(bits1,
5);
EXPECT_EQ(5u, writer.
GetNumBits()
);
EXPECT_EQ(1u, writer.
GetDataSizeBytes()
);
EXPECT_EQ("11001", writer.
GetStreamRaw()
);
}

TEST(BitWriterStringStream, WriteUnencodedU8
) {
BitWriterStringStream writer;
const uint8_t bits = 127;
writer.
WriteUnencoded(bits);
EXPECT_EQ(8u, writer.
GetNumBits()
);
EXPECT_EQ("11111110", writer.
GetStreamRaw()
);
}

TEST(BitWriterStringStream, WriteUnencodedS64
) {
BitWriterStringStream writer;
const int64_t bits = std::numeric_limits<int64_t>::min() + 7;
writer.
WriteUnencoded(bits);
EXPECT_EQ(64u, writer.
GetNumBits()
);
EXPECT_EQ("1110000000000000000000000000000000000000000000000000000000000001",
writer.
GetStreamRaw()
);
}

TEST(BitWriterStringStream, WriteMultiple
) {
BitWriterStringStream writer;

std::string expected_result;

const uint64_t b2_val = 0x4 | 0x2 | 0x40;
const std::string bits2 = BitsToStream(b2_val, 8);
writer.
WriteBits(b2_val,
8);

const uint64_t val = 0x1 | 0x2 | 0x10;
const std::string bits3 = BitsToStream(val, 8);
writer.
WriteBits(val,
8);

const std::string expected = bits2 + bits3;

EXPECT_EQ(expected
.
length(), writer
.
GetNumBits()
);
EXPECT_EQ(2u, writer.
GetDataSizeBytes()
);
EXPECT_EQ(expected, writer
.
GetStreamRaw()
);

EXPECT_EQ(PadToWord<8>(expected), BufferToStream(writer.GetDataCopy())
);
}

TEST(BitWriterWord64, Empty
) {
BitWriterWord64 writer;
EXPECT_EQ(0u, writer.
GetNumBits()
);
EXPECT_EQ(0u, writer.
GetDataSizeBytes()
);
}

TEST(BitWriterWord64, WriteBits
) {
BitWriterWord64 writer;
const uint64_t bits1 = 0x1 | 0x2 | 0x10;
writer.
WriteBits(bits1,
5);
writer.
WriteBits(bits1,
5);
writer.
WriteBits(bits1,
5);
EXPECT_EQ(15u, writer.
GetNumBits()
);
EXPECT_EQ(2u, writer.
GetDataSizeBytes()
);
}

TEST(BitWriterWord64, WriteZeroBits
) {
BitWriterWord64 writer;
writer.WriteBits(0, 0);
writer.WriteBits(1, 0);
EXPECT_EQ(0u, writer.
GetNumBits()
);
writer.WriteBits(1, 1);
writer.WriteBits(0, 0);
writer.WriteBits(0, 63);
EXPECT_EQ(64u, writer.
GetNumBits()
);
writer.WriteBits(0, 0);
writer.WriteBits(7, 3);
writer.WriteBits(0, 0);
}

TEST(BitWriterWord64, ComparisonTestWriteLotsOfBits
) {
BitWriterStringStream writer1;
BitWriterWord64 writer2(16384);

for(
uint64_t i = 0;
i < 65000; i += 25) {
writer1.
WriteBits(i,
16);
writer2.
WriteBits(i,
16);
ASSERT_EQ(writer1
.
GetNumBits(), writer2
.
GetNumBits()
);
}
}

TEST(GetLowerBits, Test
) {
EXPECT_EQ(0u, GetLowerBits <uint8_t>(255, 0));
EXPECT_EQ(1u, GetLowerBits <uint8_t>(255, 1));
EXPECT_EQ(3u, GetLowerBits <uint8_t>(255, 2));
EXPECT_EQ(7u, GetLowerBits <uint8_t>(255, 3));
EXPECT_EQ(15u, GetLowerBits <uint8_t>(255, 4));
EXPECT_EQ(31u, GetLowerBits <uint8_t>(255, 5));
EXPECT_EQ(63u, GetLowerBits <uint8_t>(255, 6));
EXPECT_EQ(127u, GetLowerBits <uint8_t>(255, 7));
EXPECT_EQ(255u, GetLowerBits <uint8_t>(255, 8));
EXPECT_EQ(0xFFu, GetLowerBits <uint32_t>(0xFFFFFFFF, 8));
EXPECT_EQ(0xFFFFu, GetLowerBits <uint32_t>(0xFFFFFFFF, 16));
EXPECT_EQ(0xFFFFFFu, GetLowerBits <uint32_t>(0xFFFFFFFF, 24));
EXPECT_EQ(0xFFFFFFu, GetLowerBits <uint64_t>(0xFFFFFFFFFFFF, 24));
EXPECT_EQ(0xFFFFFFFFFFFFFFFFu,
GetLowerBits <uint64_t>(0xFFFFFFFFFFFFFFFFu, 64));
EXPECT_EQ(StreamToBits("1010001110"),
		GetLowerBits<uint64_t>(StreamToBits("1010001110111101111111"), 10)
);
}

TEST(BitReaderFromString, FromU8
) {
std::vector<uint8_t> buffer = {
		0xAA,
		0xBB,
		0xCC,
		0xDD,
};

const std::string total_stream =
		"01010101"
		"11011101"
		"00110011"
		"10111011";

BitReaderFromString reader(buffer);

uint64_t bits = 0;
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(PadToWord<64>("01"), BitsToStream(bits)
);
EXPECT_EQ(20u, reader.
ReadBits(& bits,
20));
EXPECT_EQ(PadToWord<64>("01010111011101001100"), BitsToStream(bits)
);
EXPECT_EQ(20u, reader.
ReadBits(& bits,
20));
EXPECT_EQ(PadToWord<64>("11101110110000000000"), BitsToStream(bits)
);
EXPECT_EQ(22u, reader.
ReadBits(& bits,
30));
EXPECT_EQ(PadToWord<64>("0000000000000000000000"), BitsToStream(bits)
);
EXPECT_TRUE(reader
.
ReachedEnd()
);
}

TEST(BitReaderFromString, FromU64
) {
std::vector<uint64_t> buffer = {
		0xAAAAAAAAAAAAAAAA,
		0xBBBBBBBBBBBBBBBB,
		0xCCCCCCCCCCCCCCCC,
		0xDDDDDDDDDDDDDDDD,
};

const std::string total_stream =
		"0101010101010101010101010101010101010101010101010101010101010101"
		"1101110111011101110111011101110111011101110111011101110111011101"
		"0011001100110011001100110011001100110011001100110011001100110011"
		"1011101110111011101110111011101110111011101110111011101110111011";

BitReaderFromString reader(buffer);

uint64_t bits = 0;
size_t pos = 0;
size_t to_read = 5;
while(reader.
ReadBits(& bits, to_read
) > 0) {
EXPECT_EQ(BitsToStream(bits),
		PadToWord<64>(total_stream.substr(pos, to_read))
);
pos +=
to_read;
to_read = (to_read + 35) % 64 + 1;
}
EXPECT_TRUE(reader
.
ReachedEnd()
);
}

TEST(BitReaderWord64, ReadBitsSingleByte
) {
BitReaderWord64 reader(std::vector<uint8_t>({uint8_t(0xF0)}));
EXPECT_FALSE(reader
.
ReachedEnd()
);

uint64_t bits = 0;
EXPECT_EQ(1u, reader.
ReadBits(& bits,
1));
EXPECT_EQ(0u, bits);
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(0u, bits);
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(2u, bits);
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(3u, bits);
EXPECT_FALSE(reader
.
OnlyZeroesLeft()
);
EXPECT_FALSE(reader
.
ReachedEnd()
);
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(1u, bits);
EXPECT_TRUE(reader
.
OnlyZeroesLeft()
);
EXPECT_FALSE(reader
.
ReachedEnd()
);
EXPECT_EQ(55u, reader.
ReadBits(& bits,
64));
EXPECT_EQ(0u, bits);
EXPECT_TRUE(reader
.
ReachedEnd()
);
}

TEST(BitReaderWord64, ReadBitsTwoWords
) {
std::vector<uint64_t> buffer = {0x0000000000000001, 0x0000000000FFFFFF};

BitReaderWord64 reader(std::move(buffer));

uint64_t bits = 0;
EXPECT_EQ(1u, reader.
ReadBits(& bits,
1));
EXPECT_EQ(1u, bits);
EXPECT_EQ(62u, reader.
ReadBits(& bits,
62));
EXPECT_EQ(0u, bits);
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(2u, bits);
EXPECT_EQ(3u, reader.
ReadBits(& bits,
3));
EXPECT_EQ(7u, bits);
EXPECT_FALSE(reader
.
OnlyZeroesLeft()
);
EXPECT_EQ(32u, reader.
ReadBits(& bits,
32));
EXPECT_EQ(0xFFFFFu, bits);
EXPECT_TRUE(reader
.
OnlyZeroesLeft()
);
EXPECT_FALSE(reader
.
ReachedEnd()
);
EXPECT_EQ(28u, reader.
ReadBits(& bits,
32));
EXPECT_EQ(0u, bits);
EXPECT_TRUE(reader
.
ReachedEnd()
);
}

TEST(BitReaderFromString, ReadUnencodedU8
) {
BitReaderFromString reader("11111110");
uint8_t val = 0;
ASSERT_TRUE(reader
.
ReadUnencoded(& val)
);
EXPECT_EQ(8u, reader.
GetNumReadBits()
);
EXPECT_EQ(127, val);
}

TEST(BitReaderFromString, ReadUnencodedU16Fail
) {
BitReaderFromString reader("11111110");
uint16_t val = 0;
ASSERT_FALSE(reader
.
ReadUnencoded(& val)
);
}

TEST(BitReaderFromString, ReadUnencodedS64
) {
BitReaderFromString reader(
		"1110000000000000000000000000000000000000000000000000000000000001");
int64_t val = 0;
ASSERT_TRUE(reader
.
ReadUnencoded(& val)
);
EXPECT_EQ(64u, reader.
GetNumReadBits()
);
EXPECT_EQ (std::numeric_limits<int64_t>::min()
+ 7, val);
}

TEST(BitReaderWord64, FromU8
) {
std::vector<uint8_t> buffer = {
		0xAA,
		0xBB,
		0xCC,
		0xDD,
};

BitReaderWord64 reader(std::move(buffer));

uint64_t bits = 0;
EXPECT_EQ(2u, reader.
ReadBits(& bits,
2));
EXPECT_EQ(PadToWord<64>("01"), BitsToStream(bits)
);
EXPECT_EQ(20u, reader.
ReadBits(& bits,
20));
EXPECT_EQ(PadToWord<64>("01010111011101001100"), BitsToStream(bits)
);
EXPECT_EQ(20u, reader.
ReadBits(& bits,
20));
EXPECT_EQ(PadToWord<64>("11101110110000000000"), BitsToStream(bits)
);
EXPECT_EQ(22u, reader.
ReadBits(& bits,
30));
EXPECT_EQ(PadToWord<64>("0000000000000000000000"), BitsToStream(bits)
);
EXPECT_TRUE(reader
.
ReachedEnd()
);
}

TEST(BitReaderWord64, FromU64
) {
std::vector<uint64_t> buffer = {
		0xAAAAAAAAAAAAAAAA,
		0xBBBBBBBBBBBBBBBB,
		0xCCCCCCCCCCCCCCCC,
		0xDDDDDDDDDDDDDDDD,
};

const std::string total_stream =
		"0101010101010101010101010101010101010101010101010101010101010101"
		"1101110111011101110111011101110111011101110111011101110111011101"
		"0011001100110011001100110011001100110011001100110011001100110011"
		"1011101110111011101110111011101110111011101110111011101110111011";

BitReaderWord64 reader(std::move(buffer));

uint64_t bits = 0;
size_t pos = 0;
size_t to_read = 5;
while(reader.
ReadBits(& bits, to_read
) > 0) {
EXPECT_EQ(BitsToStream(bits),
		PadToWord<64>(total_stream.substr(pos, to_read))
);
pos +=
to_read;
to_read = (to_read + 35) % 64 + 1;
}
EXPECT_TRUE(reader
.
ReachedEnd()
);
}

TEST(BitReaderWord64, ComparisonLotsOfU8
) {
std::vector<uint8_t> buffer;
for(
uint32_t i = 0;
i < 10003; ++i) {
buffer.push_back(static_cast
<uint8_t>(i
% 255));
}

BitReaderFromString reader1(buffer);
BitReaderWord64 reader2(std::move(buffer));

uint64_t bits1 = 0, bits2 = 0;
size_t to_read = 5;
while(reader1.
ReadBits(& bits1, to_read
) > 0) {
reader2.
ReadBits(& bits2, to_read
);
EXPECT_EQ(bits1, bits2
);
to_read = (to_read + 35) % 64 + 1;
}

EXPECT_EQ(0u, reader2.
ReadBits(& bits2,
1));
}

TEST(BitReaderWord64, ComparisonLotsOfU64
) {
std::vector<uint64_t> buffer;
for(
uint64_t i = 0;
i < 1000; ++i) {
buffer.
push_back(i);
}

BitReaderFromString reader1(buffer);
BitReaderWord64 reader2(std::move(buffer));

uint64_t bits1 = 0, bits2 = 0;
size_t to_read = 5;
while(reader1.
ReadBits(& bits1, to_read
) > 0) {
reader2.
ReadBits(& bits2, to_read
);
EXPECT_EQ(bits1, bits2
);
to_read = (to_read + 35) % 64 + 1;
}

EXPECT_EQ(0u, reader2.
ReadBits(& bits2,
1));
}

TEST(ReadWriteWord64, ReadWriteLotsOfBits
) {
BitWriterWord64 writer(16384);
for(
uint64_t i = 0;
i < 65000; i += 25) {
const uint64_t num_bits = i % 64 + 1;
const uint64_t bits = i >> (64 - num_bits);
writer.
WriteBits(bits, size_t(num_bits)
);
}

BitReaderWord64 reader(writer.GetDataCopy());
for(
uint64_t i = 0;
i < 65000; i += 25) {
const uint64_t num_bits = i % 64 + 1;
const uint64_t expected_bits = i >> (64 - num_bits);
uint64_t bits = 0;
reader.
ReadBits(& bits, size_t(num_bits)
);
EXPECT_EQ(expected_bits, bits
);
}

EXPECT_TRUE(reader
.
OnlyZeroesLeft()
);
}

TEST(VariableWidthWrite, Write0U
) {
BitWriterStringStream writer;
writer.WriteVariableWidthU64(0, 2);
EXPECT_EQ("000", writer.
GetStreamRaw()
);
writer.WriteVariableWidthU32(0, 2);
EXPECT_EQ(
"000"
"000",
writer.
GetStreamRaw()
);
writer.WriteVariableWidthU16(0, 2);
EXPECT_EQ(
"000"
"000"
"000",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthWrite, WriteSmallUnsigned
) {
BitWriterStringStream writer;
writer.WriteVariableWidthU64(1, 2);
EXPECT_EQ("100", writer.
GetStreamRaw()
);
writer.WriteVariableWidthU32(2, 2);
EXPECT_EQ(
"100"
"010",
writer.
GetStreamRaw()
);
writer.WriteVariableWidthU16(3, 2);
EXPECT_EQ(
"100"
"010"
"110",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthWrite, WriteSmallSigned
) {
BitWriterStringStream writer;
writer.WriteVariableWidthS64(1, 2, 0);
EXPECT_EQ("010", writer.
GetStreamRaw()
);
writer.WriteVariableWidthS64(-1, 2, 0);
EXPECT_EQ(
"010"
"100",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthWrite, U64Val127ChunkLength7
) {
BitWriterStringStream writer;
writer.WriteVariableWidthU64(127, 7);
EXPECT_EQ(
"1111111"
"0",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthWrite, U32Val255ChunkLength7
) {
BitWriterStringStream writer;
writer.WriteVariableWidthU32(255, 7);
EXPECT_EQ(
"1111111"
"1"
"1000000"
"0",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthWrite, U16Val2ChunkLength4
) {
BitWriterStringStream writer;
writer.WriteVariableWidthU16(2, 4);
EXPECT_EQ(
"0100"
"0",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthWrite, U64ValAAAAChunkLength2
) {
BitWriterStringStream writer;
writer.WriteVariableWidthU64(0xAAAA, 2);
EXPECT_EQ(
"01"
"1"
"01"
"1"
"01"
"1"
"01"
"1"
"01"
"1"
"01"
"1"
"01"
"1"
"01"
"0",
writer.
GetStreamRaw()
);
}

TEST(VariableWidthRead, U64Val127ChunkLength7
) {
BitReaderFromString reader(
		"1111111"
		"0");
uint64_t val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU64(& val,
7));
EXPECT_EQ(127u, val);
}

TEST(VariableWidthRead, U32Val255ChunkLength7
) {
BitReaderFromString reader(
		"1111111"
		"1"
		"1000000"
		"0");
uint32_t val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU32(& val,
7));
EXPECT_EQ(255u, val);
}

TEST(VariableWidthRead, U16Val2ChunkLength4
) {
BitReaderFromString reader(
		"0100"
		"0");
uint16_t val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU16(& val,
4));
EXPECT_EQ(2u, val);
}

TEST(VariableWidthRead, U64ValAAAAChunkLength2
) {
BitReaderFromString reader(
		"01"
		"1"
		"01"
		"1"
		"01"
		"1"
		"01"
		"1"
		"01"
		"1"
		"01"
		"1"
		"01"
		"1"
		"01"
		"0");
uint64_t val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU64(& val,
2));
EXPECT_EQ(0xAAAAu, val);
}

TEST(VariableWidthRead, FailTooShort
) {
BitReaderFromString reader("00000001100000");
uint64_t val = 0;
ASSERT_FALSE(reader
.
ReadVariableWidthU64(& val,
7));
}

TEST(VariableWidthWriteRead, SingleWriteReadU64
) {
for(
uint64_t i = 0;
i < 1000000; i += 1234) {
const uint64_t val = i * i * i;
const size_t chunk_length = size_t(i % 16 + 1);

BitWriterWord64 writer;
writer.
WriteVariableWidthU64(val, chunk_length
);

BitReaderWord64 reader(writer.GetDataCopy());
uint64_t read_val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU64(& read_val, chunk_length
));

ASSERT_EQ(val, read_val
) << "Chunk length " <<
chunk_length;
}
}

TEST(VariableWidthWriteRead, SingleWriteReadS64
) {
for(
int64_t i = 0;
i < 1000000; i += 4321) {
const int64_t val = i * i * (i % 2 ? -i : i);
const size_t chunk_length = size_t(i % 16 + 1);
const size_t zigzag_exponent = size_t(i % 13);

BitWriterWord64 writer;
writer.
WriteVariableWidthS64(val, chunk_length, zigzag_exponent
);

BitReaderWord64 reader(writer.GetDataCopy());
int64_t read_val = 0;
ASSERT_TRUE(
		reader
.
ReadVariableWidthS64(& read_val, chunk_length, zigzag_exponent
));

ASSERT_EQ(val, read_val
) << "Chunk length " <<
chunk_length;
}
}

TEST(VariableWidthWriteRead, SingleWriteReadU32
) {
for(
uint32_t i = 0;
i < 100000; i += 123) {
const uint32_t val = i * i;
const size_t chunk_length = i % 16 + 1;

BitWriterWord64 writer;
writer.
WriteVariableWidthU32(val, chunk_length
);

BitReaderWord64 reader(writer.GetDataCopy());
uint32_t read_val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU32(& read_val, chunk_length
));

ASSERT_EQ(val, read_val
) << "Chunk length " <<
chunk_length;
}
}

TEST(VariableWidthWriteRead, SingleWriteReadU16
) {
for(
int i = 0;
i < 65536; i += 123) {
const uint16_t val = static_cast<int16_t>(i);
const size_t chunk_length = val % 10 + 1;

BitWriterWord64 writer;
writer.
WriteVariableWidthU16(val, chunk_length
);

BitReaderWord64 reader(writer.GetDataCopy());
uint16_t read_val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU16(& read_val, chunk_length
));

ASSERT_EQ(val, read_val
) << "Chunk length " <<
chunk_length;
}
}

TEST(VariableWidthWriteRead, SmallNumbersChunkLength4
) {
const std::vector<uint64_t> expected_values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

BitWriterWord64 writer;
for(
uint64_t val
: expected_values) {
writer.
WriteVariableWidthU64(val,
4);
}

EXPECT_EQ(50u, writer.
GetNumBits()
);

std::vector<uint64_t> actual_values;
BitReaderWord64 reader(writer.GetDataCopy());
while(!reader.
OnlyZeroesLeft()
) {
uint64_t val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU64(& val,
4));
actual_values.
push_back(val);
}

EXPECT_EQ(expected_values, actual_values
);
}

TEST(VariableWidthWriteRead, VariedNumbersChunkLength8
) {
const std::vector<uint64_t> expected_values = {1000, 0, 255, 4294967296};
const size_t kExpectedNumBits = 9 * (2 + 1 + 1 + 5);

BitWriterWord64 writer;
for(
uint64_t val
: expected_values) {
writer.
WriteVariableWidthU64(val,
8);
}

EXPECT_EQ(kExpectedNumBits, writer
.
GetNumBits()
);

std::vector<uint64_t> actual_values;
BitReaderWord64 reader(writer.GetDataCopy());
while(!reader.
OnlyZeroesLeft()
) {
uint64_t val = 0;
ASSERT_TRUE(reader
.
ReadVariableWidthU64(& val,
8));
actual_values.
push_back(val);
}

EXPECT_EQ(expected_values, actual_values
);
}

}  // namespace
}  // namespace comp
}  // namespace spvtools