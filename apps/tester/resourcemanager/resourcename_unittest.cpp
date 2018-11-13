#include "../catch.hpp"

#include "core/core.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/memstorage.h"
#include "resourcemanager/textresource.h"

#include <thread>

TEST_CASE("ResourceName bob", "[resourcemanager]")
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

TEST_CASE("ResourceName null$null", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("null$null");
	REQUIRE(test.isNull());
}


TEST_CASE("ResourceName disk$null", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("disk$null");
	REQUIRE(!test.isNull());
}

TEST_CASE("ResourceName disk$bob", "[resourcemanager]")
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

TEST_CASE("ResourceName disk$bob$0", "[resourcemanager]")
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

TEST_CASE("ResourceName ", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("");
	REQUIRE(!test.isValid());
}

TEST_CASE("ResourceName |$|$", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("|$|$");
	REQUIRE(test.isValid());
	REQUIRE(!test.isCurrentLink());
}

TEST_CASE("ResourceName |$|$0", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("|$|$0");
	REQUIRE(test.isValid());
	REQUIRE(test.isCurrentLink());
}

TEST_CASE("ResourceName $", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("$");
	REQUIRE(!test.isValid());
}

TEST_CASE("ResourceName $$", "[resourcemanager]")
{
	using namespace ResourceManager;
	ResourceName test("$$");
	REQUIRE(!test.isValid());
}

TEST_CASE("ResourceName disk bob 0", "[resourcemanager]")
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


