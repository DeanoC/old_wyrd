#include "../catch.hpp"

#include "core/core.h"
#include "shell/shell.h"
#include "vulkan/system.h"

extern Shell::ShellInterface* globalShell;

TEST_CASE( "System Init/Finish", "[Vulkan]" )
{
	auto vulkanSystem = std::make_shared<Vulkan::System>();
	REQUIRE(vulkanSystem->Init("tester",{}));
	vulkanSystem->Finish();
	vulkanSystem.reset();
}

TEST_CASE( "Shell Init/Finish", "[Vulkan]" )
{
	REQUIRE(globalShell != nullptr);
	REQUIRE(globalShell->gpusInitialised());

}