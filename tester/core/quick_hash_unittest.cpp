#include "tester/catch.hpp"

#include "core/core.h"
#include "core/quick_hash.h"
#include <thread>

TEST_CASE("QuickHash strings", "[core/quickhash]")
{
	char const cstr[] = "BobTheHero";
	std::string sstring0("BobTheHero");
	std::string sstring1{"BobTheHero"};
	std::string_view strview0(sstring0);
	std::string_view strview1(sstring1);
	REQUIRE(std::strcmp(cstr, sstring0.c_str()) == 0);
	REQUIRE(std::strcmp(cstr, sstring1.c_str()) == 0);
	REQUIRE(Core::QuickHash(cstr, strlen(cstr)) == Core::QuickHash(sstring0));
	REQUIRE(Core::QuickHash(sstring0) == Core::QuickHash(strview0));

	sstring1 = "BobThe";
	REQUIRE_FALSE(Core::QuickHash(sstring0) == Core::QuickHash(sstring1));
	strview1.remove_suffix(4);
	REQUIRE(Core::QuickHash(sstring1) == Core::QuickHash(strview1));
	REQUIRE_FALSE(Core::QuickHash(sstring0) == Core::QuickHash(strview1));
	REQUIRE_FALSE(Core::QuickHash(strview0) == Core::QuickHash(strview1));
}
