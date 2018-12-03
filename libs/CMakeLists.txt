# CMake version
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# global packages and options (mostly for thirdparties)
find_package(vulkan REQUIRED)

#--------- Global defination and include
set( wyrd_DEFINITIONS
		-D__TBB_NO_IMPLICIT_LINKAGE=1
		-D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1
		-D_CRT_SECURE_NO_WARNINGS -D__STDC_FORMAT_MACROS
		-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
		-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING=1
		-DCRC32C_STATIC
		)

set( wyrd_DEFINITIONS ${wyrd_DEFINITIONS} PARENT_SCOPE )

set( wyrd_INCLUDES
		${Vulkan_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tbb/include
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt/include
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppfs/include
		)
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
add_subdirectory(level6)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})
add_library(wyrd_static dummy.cpp)

set( wyrd_INCLUDES ${wyrd_INCLUDES} PARENT_SCOPE )
target_link_libraries(wyrd_static thirdparty)
foreach(LIB ${ALL_WYRD_LIBS})
	target_link_libraries(wyrd_static ${LIB})
endforeach(LIB)
