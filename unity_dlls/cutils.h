//
// Created by Computer on 11/09/2018.
//

#ifndef NATIVESNAPSHOT_CUTILS_H
#define NATIVESNAPSHOT_CUTILS_H

#include "core/core.h"
namespace Core { struct Blob; }

// this structure is directly mappable in unity or native code
// which for many mesh cases (collision etc.) is good enough
struct SimpleMesh
{
	uint32_t positionCount;
	float *positions; // 3 * float per position Unity style (x left y in and z up)
	uint32_t triangleCount;
	uint32_t *triangleIndices; // 3 * uint32_t per triangle
};

struct CUtilsInterface
{
	// crc32c functions
	// computes crc32c from an array, first param should be 0 or value from previous call
	// Use CPU HW acceleration where available
	CAPI uint32_t(*ComputeCRC32C)(uint32_t crc, uint8_t const *input, uint64_t length);
	// returns the crc32c of a simple mesh
	CAPI uint32_t(*ComputeCRC32COfSimpleMesh)(uint32_t crc_, SimpleMesh *simpleMesh);
	// returns the crc32c of a string
	CAPI uint32_t(*ComputeCRC32COfString)(uint32_t crc, char const *input, uint64_t length);

	// bundle functions
	CAPI uint64_t(*FetchBundleUserData)(char const *bundlePath);

	// blob functions
	CAPI bool(*AllocBlob)(uint64_t size_, Core::Blob *out_);
	CAPI void(*FreeBlob)(Core::Blob *in_);
};
#if !defined(USING_STATIC_LIBS)
EXPORT void* GetInterface();
#else
CUtilsInterface* CUtils();
#endif

#endif //NATIVESNAPSHOT_CUTILS_H
