option(BUILD_GFXTEST "Build the gfxtest apps." ON)
option(BUILD_PP_VIEWER "Build the pp_viewer apps." ON)

if(BUILD_GFXTEST)
	add_subdirectory(gfxtest)
endif(BUILD_GFXTEST)

if(BUILD_PP_VIEWER)
	add_subdirectory(pp_viewer)
endif(BUILD_PP_VIEWER)

if(BUILD_TESTER)
	add_subdirectory(tester)
endif(BUILD_TESTER)

if (BUILD_TOOLS)
	#	add_subdirectory(tools/vulkaninfo)
endif (BUILD_TOOLS)

