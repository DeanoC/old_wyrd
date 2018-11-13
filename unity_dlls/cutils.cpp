#if !defined(USING_STATIC_LIBS)
#define STB_IMAGE_IMPLEMENTATION
#define LOGURU_IMPLEMENTATION 1
#endif

#include "core/core.h"
#include "core/blob.h"
#include "crc32c/crc32c.h"
#include "binny/bundle.h"
#include <fstream>
#include "cutils.h"


// crc32c function (HW accelerated where possible)
CAPI uint32_t ComputeCRC32C(uint32_t crc, uint8_t const* input, uint64_t length)
{
	assert(length < std::numeric_limits<size_t>::max());
	return crc32c_append(crc, input, length);
}

// returns the crc32c of a simple mesh
CAPI uint32_t ComputeCRC32COfSimpleMesh(uint32_t crc_, SimpleMesh *simpleMesh)
{
	crc_ = crc32c_append(crc_, (uint8_t*)simpleMesh->positions, simpleMesh->positionCount * sizeof(float) * 3);
	crc_ = crc32c_append(crc_, (uint8_t*)simpleMesh->triangleIndices, simpleMesh->triangleCount * sizeof(uint32_t) * 3);

	return crc_;
}
// returns the crc32c of a string
CAPI uint32_t ComputeCRC32COfString(uint32_t crc, char const* input, uint64_t length)
{
	assert(length < std::numeric_limits<size_t>::max());
	return crc32c_append(crc, (uint8_t*) input, length);
}

CAPI uint64_t FetchBundleUserData(char const* bundlePath)
{
	using namespace Binny;
	std::ifstream in(bundlePath, std::ifstream::in | std::ifstream::binary);
	Bundle bundle(&malloc, &free, &malloc, &free, in);

	auto const ret = bundle.peekAtHeader();

	if (ret.first == Bundle::ErrorCode::Okay)
	{
		return ret.second;
	}

	return ~0;
}

CAPI bool AllocBlob(uint64_t size_, Core::Blob* out_)
{
	if(out_ == nullptr) return false;
	return Core::Blob::Create(size_, out_);
}

CAPI void FreeBlob(Core::Blob* in_)
{
	if(in_ == nullptr) return;
	Core::Blob::Free(in_);
}

static CUtilsInterface Interface;

#if !defined(USING_STATIC_LIBS)
EXPORT void* GetInterface()
#else
CUtilsInterface* CUtils()
#endif
{
	if(Interface.ComputeCRC32C == nullptr)
	{
		Interface.ComputeCRC32C = &ComputeCRC32C;
		Interface.ComputeCRC32COfSimpleMesh = &ComputeCRC32COfSimpleMesh;
		Interface.ComputeCRC32COfString = &ComputeCRC32COfString;
		Interface.FetchBundleUserData = &FetchBundleUserData;
		Interface.AllocBlob = &AllocBlob;
		Interface.FreeBlob = &FreeBlob;
	}
	return &Interface;
}