option(BUILD_GFXTEST "Build the gfxtest apps." ON)
option(BUILD_REPLAY "Build the replay app." ON)

set(apps_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_GFXTEST)
	add_subdirectory(gfxtest)
endif(BUILD_GFXTEST)

if(BUILD_REPLAY)
	add_subdirectory(replay)
endif(BUILD_REPLAY)

if(BUILD_TESTER)
	add_subdirectory(tester)
endif(BUILD_TESTER)

if (BUILD_TOOLS)
	#	add_subdirectory(tools/vulkaninfo)
endif (BUILD_TOOLS)

