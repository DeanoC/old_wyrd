#include "../catch.hpp"
#include "core/core.h"
#include "binny/bundle.h"
#include "binny/bundlewriter.h"
#include <vector>
#include <string_view>

TEST_CASE( "Bundle chunks write/read", "[Binny]" )
{
	using namespace Binny;
	using namespace std::string_view_literals;
	std::string textBinify, log;
	BundleWriter testBundle;
//	testBundle.setLogBinifyText();

	testBundle.addRawTextChunk( "test_chunk2", "TEST"_bundle_id, 0, 0,
								0, {},
								"bobbobobobobobobboboboboboboboboboboboobobobobobobobobobobobobobboboobboboboobboboboboboobboboobbobobobobobobobooosdfsdfdsfoobobEND\n" );
	testBundle.addRawTextChunk( "test_chunk3", "TEST"_bundle_id, 0, 1,
								0, {},
								"bAsadasdasdobobodsfjldjkladsjfdlskjfldasfjdlkfjdlkfjepi4-359345mvavdsaf435oboboboboobobobobobobobobobobobobobboboobboboboobboboboboboobboboobbobobobobobobobooosdfsdfdsfoobobEND\n" );
	testBundle.addChunk( "test_chunk4", "TEST"_bundle_id, 0, 2,
						 0, {},
						 []( WriteHelper& o )
						 {
							 o.add_string( "bob" );
							 o.add_string( "bob" );
							 o.add_string( "bob2" );
							 o.add_string( "bob3" );
						 } );

	std::vector<uint8_t> out;
	testBundle.build( 0, out );

	std::string outStr;
	outStr.resize( out.size());
	std::memcpy( outStr.data(), out.data(), out.size());

	std::istringstream in( outStr );

	std::vector<Bundle::ChunkHandler> handlers = {
		{{	"TEST"_bundle_id, 0, 0,
			[]( std::string_view subObject_, int stage_, uint16_t majorVersion_, uint16_t minorVersion_, std::shared_ptr<void> ptr_ ) -> bool
			{
				if(stage_ != 0) return false;
				if(majorVersion_ != 0) return false;
				if(minorVersion_ > 3) return false;

				if(minorVersion_ == 0)
				{
					char const *strings = (char const *) ptr_.get();
					REQUIRE( strcmp( strings,
									"bobbobobobobobobboboboboboboboboboboboobobobobobobobobobobobobobboboobboboboobboboboboboobboboobbobobobobobobobooosdfsdfdsfoobobEND\n" ) ==
							0 );
				}
				if(minorVersion_ == 1)
				{
					char const *strings = (char const *) ptr_.get();
					REQUIRE( strcmp( strings,
									"bAsadasdasdobobodsfjldjkladsjfdlskjfldasfjdlkfjdlkfjepi4-359345mvavdsaf435oboboboboobobobobobobobobobobobobobboboobboboboobboboboboboobboboobbobobobobobobobooosdfsdfdsfoobobEND\n" ) ==
							0 );
				}
				if(minorVersion_ == 2)
				{
					char const **strings = (char const **) ptr_.get();
					REQUIRE( strcmp( strings[0], "bob" ) == 0 );
					REQUIRE( strcmp( strings[1], "bob" ) == 0 );
					REQUIRE( strings[0] == strings[1] );
					REQUIRE( strcmp( strings[2], "bob2" ) == 0 );
					REQUIRE( strcmp( strings[3], "bob3" ) == 0 );
				}
				return true;
			},
			[](int, void*) -> void
			{
			}
	 	}}
	};

	Bundle testRead( &malloc, &free, &malloc, &free, in );
	auto result = testRead.read(""sv, handlers);
	REQUIRE(result.first != Bundle::ErrorCode::Okay);
}
