#include "tester/catch.hpp"

#include "core/core.h"
#include "core/blob.h"
#include "cutils.h"

TEST_CASE( "CUtils Blobs", "[CUtils/Blob]" )
{
	using namespace Core;
	Blob stackBlob;
	Blob* blob0 = &stackBlob;
	REQUIRE(CUtils()->AllocBlob(10, blob0));
	REQUIRE(blob0->size == 10);
	REQUIRE(blob0->nativeData != nullptr);
	CUtils()->FreeBlob(blob0);
	REQUIRE(blob0->size == 0);
	REQUIRE(blob0->nativeData == nullptr);

	// allocating 0 bytes still produces a valid pointer
	REQUIRE(CUtils()->AllocBlob(0, blob0));
	REQUIRE(blob0->size == 0);
	REQUIRE(blob0->nativeData != nullptr);
	CUtils()->FreeBlob(blob0);
	REQUIRE(blob0->size == 0);
	REQUIRE(blob0->nativeData == nullptr);


}