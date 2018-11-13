#include "../catch.hpp"

#include "core/core.h"
#include "core/freelist.h"
#include <thread>

TEST_CASE( "IntrusiveFreeList<uint64_t, uint64_t>", "[core/freelist]" )
{
	using namespace Core;
	IntrusiveFreeList<uint64_t, uint64_t> freeList( 2 );
	REQUIRE( freeList.empty());
	REQUIRE( freeList.size() == 2 );
	REQUIRE( freeList.alloc() == 0 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.empty() == false );
	REQUIRE( freeList.size() == 2 );
	freeList.erase( 1 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.alloc() == 2 );
	REQUIRE( freeList.size() > 2 );
	freeList.erase( 1 );
	freeList.erase( 2 );
	freeList.erase( 0 );
	REQUIRE( freeList.empty());
	REQUIRE( freeList.alloc() == 0 );
	REQUIRE( freeList.alloc() == 2 );
	REQUIRE( freeList.alloc() == 1 );
	uint64_t i0 = freeList.push( 10 );
	uint64_t i1 = freeList.push( 20 );
	REQUIRE( freeList[i0] == 10 );
	REQUIRE( freeList[i1] == 20 );
	freeList.erase( i0 );
	REQUIRE( freeList[i1] == 20 );

}

TEST_CASE( "ExplicitFreeList<uint64_t, uint64_t>", "[core/freelist]" )
{
	using namespace Core;
	ExplicitFreeList<uint64_t, uint64_t> freeList( 2 );
	REQUIRE( freeList.empty());
	REQUIRE( freeList.size() == 2 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.alloc() == 0 );
	REQUIRE( freeList.empty() == false );
	REQUIRE( freeList.size() == 2 );
	freeList.erase( 1 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.alloc() == 4 );
	REQUIRE( freeList.size() > 2 );
	freeList.erase( 1 );
	freeList.erase( 4 );
	freeList.erase( 0 );
	REQUIRE( freeList.empty());
	REQUIRE( freeList.alloc() == 0 );
	REQUIRE( freeList.alloc() == 4 );
	REQUIRE( freeList.alloc() == 1 );
	uint64_t i0 = freeList.push( 10 );
	uint64_t i1 = freeList.push( 20 );
	REQUIRE( freeList[i0] == 10 );
	REQUIRE( freeList[i1] == 20 );
	freeList.erase( i0 );
	REQUIRE( freeList[i1] == 20 );

}

TEST_CASE( "MTFreeList<uint64_t, uint64_t>", "[core/freelist]" )
{
	using namespace Core;
	MTFreeList<uint64_t, uint64_t> freeList( 2 );
	REQUIRE( freeList.empty());
	REQUIRE( freeList.alloc() == 0 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.empty() == false );
	REQUIRE( freeList.size() == 2 );
	freeList.erase( 1 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.alloc() == 2 );
	REQUIRE( freeList.size() > 2 );
	freeList.erase( 1 );
	freeList.erase( 2 );
	freeList.erase( 0 );
	REQUIRE( freeList.empty());
	REQUIRE( freeList.alloc() == 3 );
	REQUIRE( freeList.alloc() == 4 );
	REQUIRE( freeList.alloc() == 1 );
	REQUIRE( freeList.alloc() == 2 );
	REQUIRE( freeList.alloc() == 0 );

	uint64_t i0 = freeList.push( 10 );
	uint64_t i1 = freeList.push( 20 );
	REQUIRE( freeList[i0] == 10 );
	REQUIRE( freeList[i1] == 20 );
	freeList.erase( i0 );
	REQUIRE( freeList[i1] == 20 );


	// MT threaded tests
	constexpr int ThreadCount = 100;
	std::thread threads[ThreadCount];
	// spawn 1000 threads: push, pop etc. to try and break things
	for(int i = 0; i < ThreadCount; ++i)
	{
		threads[i] = std::thread( [&freeList]( int thread_id )
								  {
									  if(thread_id & 0x1)
									  {
										  auto it0 = freeList.push( thread_id );
										  REQUIRE( freeList[it0] == thread_id );
										  auto it1 = freeList.push( thread_id );
										  REQUIRE( freeList[it0] == thread_id );
										  REQUIRE( freeList[it1] == thread_id );
									  } else
									  {
										  auto it0 = freeList.push( thread_id );
										  REQUIRE( freeList[it0] == thread_id );
										  freeList.erase( it0 );
									  }
								  }, i );
	}

	for(auto& th : threads) th.join();

}