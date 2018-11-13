# global packages and options (mostly for thirdparties)
find_package(vulkan REQUIRED)

option(ENKITS_BUILD_C_INTERFACE "" OFF )
option(ENKITS_BUILD_EXAMPLES "" OFF )
option(TBB_BUILD_SHARED "" OFF)
option(TBB_BUILD_STATIC "" ON)
option(TBB_BUILD_TBBMALLOC "" ON)
option(TBB_BUILD_TBBMALLOC_PROXY "" OFF)
option(TBB_BUILD_TESTS "" OFF)
option(SHADERC_SKIP_INSTALL "" ON)
option(SHADERC_SKIP_TESTS "" ON)
option(SHADERC_ENABLE_NV_EXTENSIONS "" ON)

#--------- Global defination and include
set( wyrd_DEFINITIONS
		-D__TBB_NO_IMPLICIT_LINKAGE=1
		-D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1
		-D_CRT_SECURE_NO_WARNINGS -D__STDC_FORMAT_MACROS
		-D__TBB_SOURCE_DIRECTLY_INCLUDED=1
		-DCRC32C_STATIC
		)
set( wyrd_DEFINITIONS ${wyrd_DEFINITIONS} PARENT_SCOPE )

set( wyrd_INCLUDES
		${Vulkan_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tbb/include
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt/include
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/shaderc/libshaderc/include )

add_subdirectory(thirdparty)

#------------ Wyrd's own sources ------------
# No library can access any other library above its level
# strict levels (i.e. level 1 library can only access librarys of level 1 and 0 and so on)
set( LEVEL0_LIBS core binify)
foreach(LIB ${LEVEL0_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( ALL_WYRD_LIBS ${LEVEL0_LIBS} )
add_subdirectory(level1)
add_subdirectory(level2)
add_subdirectory(level3)
add_subdirectory(level4)
add_subdirectory(level5)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})
add_library(wyrd_static dummy.cpp)

set( wyrd_INCLUDES ${wyrd_INCLUDES} PARENT_SCOPE )
target_link_libraries(wyrd_static thirdparty)
foreach(LIB ${ALL_WYRD_LIBS})
	target_link_libraries(wyrd_static ${LIB})
endforeach(LIB)
