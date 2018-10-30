#include "../catch.hpp"

#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/memstorage.h"
#include "resourcemanager/textresource.h"

#include <thread>

TEST_CASE("Resource Manager create/destroy", "[resourcemanager]")
{
	using namespace ResourceManager;
	for(int i = 0; i < 100; ++i)
	{
		auto rm = ResourceMan::Create();
		REQUIRE(rm);
		rm.reset();

	}
}

SCENARIO("Resource Manager has mem storage", "[resourcemanager]")
{
	using namespace ResourceManager;
	using namespace Binny;
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	constexpr char TestTxt0[] = "Bob the Hero text test";
	std::string testText = std::string(TestTxt0);

	GIVEN("memory bundles")
	{
		auto rm = ResourceManager::ResourceMan::Create();
		REQUIRE(rm);
		auto memstorage = std::make_shared<MemStorage>();
		REQUIRE(memstorage);
		rm->registerStorageHandler(memstorage);
		rm->registerResourceHandler("TEST"_resource_id,
									{0, [](int stage_, ResourceManager::ResolverInterface, uint16_t majorVersion_,
										   uint16_t minorVersion_, std::shared_ptr<void> ptr_) -> bool
									{
										if(majorVersion_ != 0) return false;
										if(minorVersion_ != 0) return false;
										if(stage_ != 0) return false;

										return true;
									}, [](int, void *) -> bool
									 { return true; }});

		bool ok0 = memstorage->addMemory("test"s, "TEST"_resource_id, 0, 0, TestTxt0, strlen(TestTxt0) + 1);
		REQUIRE(ok0);
		bool ok1 = memstorage->addMemory("test2"s, "TEST"_resource_id, 0, 0, TestTxt0, strlen(TestTxt0) + 1);
		REQUIRE(ok1);

		WHEN("resource handles are opened")
		{
			auto handle0 = rm->openResourceByName<"TEST"_resource_id>("mem$test"sv);
			auto handle1 = rm->openResourceByName<"TEST"_resource_id>("mem$test2"sv);
			WHEN("resources are acquired")
			{
				auto resource0 = handle0.acquire();
				REQUIRE(resource0);
				auto resource1 = handle1.acquire();
				REQUIRE(resource1);
				auto txt0 = (char const *) resource0.get();
				auto txt1 = (char const *) resource1.get();
				REQUIRE(testText == std::string(txt0));
				REQUIRE(testText == std::string(txt1));
				REQUIRE(std::string(txt0) == std::string(txt1));
				WHEN("resources are release")
				{
					resource0.reset();
					REQUIRE(!resource0);
					resource1.reset();
					REQUIRE(!resource1);
				}
			}
		} WHEN("TextResource is added and used")
		{
			auto stage = rm->registerNextResourceHandler(TextResource::Id,
				 {10, [&testText](int stage_, ResourceManager::ResolverInterface,
								  uint16_t majorVersion_,
								  uint16_t minorVersion_,
								  ResourceBase::Ptr ptr_) -> bool
				 {
					 if(majorVersion_ != 0) return false;
					 if(minorVersion_ != 0) return false;
					 if(stage_ != 1) return false;

					 auto txtr = std::static_pointer_cast<TextResource>(ptr_);
					 std::string txt = txtr->getText();
					 REQUIRE(txt == testText);
					 uint8_t* bytePtr = txtr->getStage<uint8_t>(stage_);
					 for(auto i = 0u; i < 10; ++i)
					 {
						 REQUIRE(bytePtr[i] == 0xB1);
					 }

					 std::memset(bytePtr, 0xAA, 10);

					 return true;
				 }, [](int, void *) -> bool
				  { return true; }});

			REQUIRE(stage == 1);

			bool okay0 = memstorage->addMemory("testr"s, TextResource::Id, 0, 0, TestTxt0, strlen(TestTxt0) + 1);
			REQUIRE(okay0);
			bool okay1 = memstorage->addMemory("testr2"s, TextResource::Id, 0, 0, TestTxt0, strlen(TestTxt0) + 1);
			REQUIRE(okay1);

			auto h0 = rm->openResourceByName<TextResource::Id>("mem$testr"sv);
			auto h1 = rm->openResourceByName<TextResource::Id>("mem$testr2"sv);

			auto r0 = h0.acquire<TextResource>();
			REQUIRE(r0);
			auto r1 = h1.acquire<TextResource>();
			REQUIRE(r1);
			auto t0 = r0->getText();
			REQUIRE(t0);
			auto t1 = r0->getText();
			REQUIRE(t1);
			REQUIRE(std::strcmp(t0, t1) == 0);

			REQUIRE(r0->getStage<void>(1) != nullptr);
			REQUIRE(r1->getStage<void>(1) != nullptr);
			auto exm0 = r0->getStage<uint8_t>(1);
			auto exm1 = r1->getStage<uint8_t>(1);
			for(auto j = 0u; j < 10u; ++j)
			{
				REQUIRE(exm0[j] == 0xAA);
				REQUIRE(exm1[j] == 0xAA);
			}
		}
	}
}

