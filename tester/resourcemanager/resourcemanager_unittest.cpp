#include "tester/catch.hpp"

#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/memstorage.h"

#include <thread>

TEST_CASE( "ResourceManager::splitResourceName bob", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("bob");
	REQUIRE(test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE(storage == "disk");
	REQUIRE(name == "bob");
	REQUIRE(subobject.empty());

}
TEST_CASE( "ResourceManager::splitResourceName disk:bob", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("disk$bob");
	REQUIRE(test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE(storage == "disk");
	REQUIRE(name == "bob");
	REQUIRE(subobject.empty());
}

TEST_CASE( "ResourceManager::splitResourceName disk:bob:0", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("disk$bob$0");
	REQUIRE(test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE(storage == "disk");
	REQUIRE(name == "bob");
	REQUIRE(subobject == "0");
}

TEST_CASE( "ResourceManager::splitResourceName ", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("");
	REQUIRE(!test.isValid());
}

TEST_CASE( "ResourceManager::splitResourceName $", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("$");
	REQUIRE(!test.isValid());
}
TEST_CASE( "ResourceManager::splitResourceName $$", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("$$");
	REQUIRE(!test.isValid());
}

TEST_CASE( "ResourceManager::splitResourceName disk bob 0", "[resourcemanager]" )
{
	using namespace ResourceManager;
	ResourceName test("disk", "bob", "0");
	REQUIRE(test.isValid());
	std::string_view storage = test.getStorage();
	std::string_view name = test.getName();
	std::string_view subobject = test.getSubObject();
	REQUIRE(storage == "disk");
	REQUIRE(name == "bob");
	REQUIRE(subobject == "0");
}
TEST_CASE( "Resource Manager create/destroy", "[resourcemanager]")
{
	using namespace ResourceManager;
	auto rm = ResourceMan::Create();
	REQUIRE(rm);
	rm.reset();

}
struct TestFunc : public ResourceManager::IResourceFuncs
{
	uint32_t getType() const override
	{
		using namespace Binny;
		using namespace std::string_literals;
		return "TEST"_bundle_id;
	}

	InitFunc getInitFunc() const override
	{
		return [](uint16_t majorVersion_, uint16_t minorVersion_, ResourceManager::ResourceBase::Ptr ptr)
		{
			REQUIRE(majorVersion_ == 0);
			REQUIRE(minorVersion_ == 0);

			return true;
		};
	}

	DestroyFunc getDestroyFunc() const override
	{
		return [](void*) { return true; };
	}
};
SCENARIO( "Resource Manager has mem storage", "[resourcemanager]")
{
	using namespace ResourceManager;
	using namespace Binny;
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	static const char TestTxt0[] = "Bob the Hero text test";
	std::shared_ptr<void> txt0((void*)TestTxt0, [](void*){});

	auto testFunc = std::make_shared<TestFunc>();

	GIVEN("memory bundles")
	{
		auto rm = ResourceManager::ResourceMan::Create();
		REQUIRE(rm);
		auto memstorage = std::make_shared<MemStorage>();
		REQUIRE(memstorage);
		rm->registerStorageHandler(memstorage);
		rm->registerResourceType(testFunc);

		bool ok0 = memstorage->addMemory( "test"s, "TEST"_bundle_id, 0, 0, txt0 );
		REQUIRE(ok0);
		bool ok1 = memstorage->addStaticMemory("test2"s, "TEST"_bundle_id, 0, 0, TestTxt0);
		REQUIRE(ok1);

		WHEN("resource handles are opened")
		{
			auto handle0 = rm->openResourceByName<"TEST"_bundle_id>("mem$test"sv);
			auto handle1 = rm->openResourceByName<"TEST"_bundle_id>("mem$test2"sv);
			WHEN("resources are acquired")
			{
				auto resource0 = handle0.acquire();
				REQUIRE(resource0);
				auto resource1 = handle1.acquire();
				REQUIRE(resource1);
				auto txt0 = (char const*) resource0.get();
				auto txt1 = (char const*) resource1.get();
				REQUIRE(std::string(txt0) == std::string(txt1));
				WHEN("resources are release")
				{
					resource0.reset();
					REQUIRE(!resource0);
					resource1.reset();
					REQUIRE(!resource1);
				}
			}

			}
	}
}