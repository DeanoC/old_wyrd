#include "tester/catch.hpp"

#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/memstorage.h"
#include "resourcemanager/textresource.h"

#include <thread>

TEST_CASE( "ResourceManager::splitResourceName bob", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "bob" );
	REQUIRE( test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE( storage == "disk" );
	REQUIRE( name == "bob" );
	REQUIRE( subobject.empty());

}

TEST_CASE( "ResourceManager::splitResourceName disk:bob", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "disk$bob" );
	REQUIRE( test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE( storage == "disk" );
	REQUIRE( name == "bob" );
	REQUIRE( subobject.empty());
}

TEST_CASE( "ResourceManager::splitResourceName disk:bob:0", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "disk$bob$0" );
	REQUIRE( test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE( storage == "disk" );
	REQUIRE( name == "bob" );
	REQUIRE( subobject == "0" );
}

TEST_CASE( "ResourceManager::splitResourceName ", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "" );
	REQUIRE( !test.isValid());
}

TEST_CASE( "ResourceManager::splitResourceName $", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "$" );
	REQUIRE( !test.isValid());
}

TEST_CASE( "ResourceManager::splitResourceName $$", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "$$" );
	REQUIRE( !test.isValid());
}

TEST_CASE( "ResourceManager::splitResourceName disk bob 0", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test( "disk", "bob", "0" );
	REQUIRE( test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE( storage == "disk" );
	REQUIRE( name == "bob" );
	REQUIRE( subobject == "0" );
}

TEST_CASE( "Resource Manager create/destroy", "[resourcemanager]" )
{
	using namespace ResourceManager;
	for(int i = 0; i < 100; ++i)
	{
		auto rm = ResourceMan::Create();
		REQUIRE( rm );
		rm.reset();

	}
}

SCENARIO( "Resource Manager has mem storage", "[resourcemanager]" )
{
	using namespace ResourceManager;
	using namespace Binny;
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	constexpr char TestTxt0[] = "Bob the Hero text test";

	GIVEN( "memory bundles" )
	{
		auto rm = ResourceManager::ResourceMan::Create();
		REQUIRE( rm );
		auto memstorage = std::make_shared<MemStorage>();
		REQUIRE( memstorage );
		rm->registerStorageHandler( memstorage );
		rm->registerResourceHandler( "TEST"_resource_id, 0, { 0,
				 [](uint16_t majorVersion_, uint16_t minorVersion_, std::shared_ptr<void> ptr_ ) -> bool
				 {
					 if(majorVersion_ != 0) return false;
					 if(minorVersion_ != 0) return false;

					 return true;
				 },
				 []( void * ) -> bool
				 {
				 	return true;
				 }
		 });

		bool ok0 = memstorage->addMemory( "test"s, "TEST"_resource_id, 0, 0, TestTxt0, strlen(TestTxt0)+1 );
		REQUIRE( ok0 );
		bool ok1 = memstorage->addMemory( "test2"s, "TEST"_resource_id, 0, 0, TestTxt0, strlen(TestTxt0)+1 );
		REQUIRE( ok1 );

		WHEN( "resource handles are opened" )
		{
			auto handle0 = rm->openResourceByName<"TEST"_resource_id>( "mem$test"sv );
			auto handle1 = rm->openResourceByName<"TEST"_resource_id>( "mem$test2"sv );
			WHEN( "resources are acquired" )
			{
				auto resource0 = handle0.acquire();
				REQUIRE( resource0 );
				auto resource1 = handle1.acquire();
				REQUIRE( resource1 );
				auto txt0 = (char const *) resource0.get();
				auto txt1 = (char const *) resource1.get();
				REQUIRE( std::string(txt0) == std::string(txt1));
				WHEN( "resources are release" )
				{
					resource0.reset();
					REQUIRE( !resource0 );
					resource1.reset();
					REQUIRE( !resource1 );
				}
			}
		}
		WHEN("TextResource is added and used")
		{
			rm->registerResourceHandler( TextResource::Id, 0, { 10,
			  [](uint16_t majorVersion_, uint16_t minorVersion_, std::shared_ptr<void> ptr_ ) -> bool
			  {
				  if(majorVersion_ != 0) return false;
				  if(minorVersion_ != 0) return false;

				  return true;
			  },
			  []( void * ) -> bool
			  {
				  return true;
			  }
			});

			bool okay0 = memstorage->addMemory( "testr"s, TextResource::Id, 0, 0, TestTxt0, strlen(TestTxt0)+1 );
			REQUIRE( okay0 );
			bool okay1 = memstorage->addMemory( "testr2"s, TextResource::Id, 0, 0, TestTxt0, strlen(TestTxt0)+1 );
			REQUIRE( okay1 );

			auto h0 = rm->openResourceByName<TextResource::Id>( "mem$testr"sv );
			auto h1 = rm->openResourceByName<TextResource::Id>( "mem$testr2"sv );

			auto r0 = h0.acquire<TextResource>();
			REQUIRE( r0 );
			auto r1 = h1.acquire<TextResource>();
			REQUIRE( r1 );
			auto t0 = r0->getText();
			REQUIRE(t0);
			auto t1 = r0->getText();
			REQUIRE(t1);
			REQUIRE(std::strcmp(t0,t1)==0);
			REQUIRE(r0->getExtraMemPtr(0) != nullptr);
			REQUIRE(r1->getExtraMemPtr(0) != nullptr);
			for(auto i = 1u; i < ResourceMan::MaxHandlerStages; ++i)
			{
				REQUIRE(r0->getExtraMemPtr(i) == nullptr);
				REQUIRE(r1->getExtraMemPtr(i) == nullptr);
			}
		}
	}
}

