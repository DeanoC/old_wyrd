#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define LOGURU_IMPLEMENTATION 1
#define CX_ERROR_IMPLEMENTATION 1

#include "catch.hpp"
#include "core/core.h"
#include "cityhash/city.h"

TEST_CASE( "CityHash128 100 bytes", "[CityHash]" )
{
	using namespace CityHash;
	char smalldata[100];
	for(int i = 0; i < 100; ++i)
	{
		smalldata[i] = i;
	}

	uint128 result0 = CityHash::Hash128( smalldata, 100 );
	for(int i = 0; i < 100; ++i)
	{
		smalldata[i] = 1;
		uint128 result1 = CityHash::Hash128( smalldata, 100 );
		REQUIRE( result0.first != result1.first );
		REQUIRE( result0.second != result1.second );
	}
}

TEST_CASE( "CityHash128 1000 bytes", "[CityHash]" )
{
	using namespace CityHash;
	char data[1000];
	for(int i = 0; i < 1000; ++i)
	{
		data[i] = i;
	}

	uint128 result0 = CityHash::Hash128( data, 1000 );
	for(int i = 0; i < 1000; ++i)
	{
		data[i] = 1;
		uint128 result1 = CityHash::Hash128( data, 1000 );
		REQUIRE( result0.first != result1.first );
		REQUIRE( result0.second != result1.second );
	}
}