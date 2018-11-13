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
		-DCRC32C_STATIC
		)
set( wyrd_DEFINITIONS ${wyrd_DEFINITIONS} PARENT_SCOPE )

set( wyrd_INCLUDES
		${Vulkan_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/tbb/include
		${CMAKE_CURRENT_SOURCE_DIR}/fmt/include
		${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libshaderc/include
		${CMAKE_CURRENT_SOURCE_DIR}/meshops/VHACD_Lib/inc
		)

set( wyrd_INCLUDES ${wyrd_INCLUDES} PARENT_SCOPE )
add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

#--------- Wyrd's third party sources
# TODO move to seperate cmakelist
set( CRC32C_SRC
		crc32c/crc32c.cpp
		crc32c/crc32c.h)

set( CITYHASH_SRC
		cityhash/city.cc
		cityhash/city.h
		)
set( CX_SRC
		cx/cx_algorithm.h
		cx/cx_array.h
		cx/cx_counter.h
		cx/cx_error.h
		cx/cx_fnv1.h
		cx/cx_guid.h
		cx/cx_math.h
		cx/cx_md5.h
		cx/cx_murmur3.h
		cx/cx_numeric.h
		cx/cx_pcg32.h
		cx/cx_sha256.h
		cx/cx_strenc.h
		cx/cx_typeid.h
		cx/cx_utils.h
		)

set( FMT_SRC
		fmt/src/format.cc
		fmt/src/posix.cc
		)

set( LZ4_SRC
		lz4/lz4.c
		lz4/lz4.h
		lz4/lz4frame.c
		lz4/lz4frame.h
		lz4/lz4frame_static.h
		lz4/lz4hc.c
		lz4/lz4hc.h
		lz4/xxhash.c
		lz4/xxhash.h
		)

set(IMGUI_SRC
		imgui/imconfig.h
		imgui/imgui.h
		imgui/imgui.cpp
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_widgets.cpp
		)

set( THIRDPARTY_LIBS enkiTS shaderc)
foreach(LIB ${THIRDPARTY_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)
add_subdirectory(tbb)
set( THIRDPARTY_LIBS ${THIRDPARTY_LIBS} tbb_static tbbmalloc_static)

add_library( thirdparty STATIC
		${CRC32C_SRC}
		${CITYHASH_SRC}
		${CX_SRC}
		${FMT_SRC}
		${LZ4_SRC}
		${IMGUI_SRC}
		)
target_link_libraries(thirdparty ${THIRDPARTY_LIBS})

#------------ Wyrd's own sources ------------
# TODO enforce library level strictness for includes
# No library should access any other library at its level or above
# strict levels (i.e. level 1 library can only access librarys of level 0 and so on)

set( LEVEL0_LIBS core binify)
foreach(LIB ${LEVEL0_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( LEVEL1_LIBS binny math input)
foreach(LIB ${LEVEL1_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( LEVEL2_LIBS resourcemanager geometry)
foreach(LIB ${LEVEL2_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( LEVEL3_LIBS tacticalmap render meshmod)
foreach(LIB ${LEVEL3_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( LEVEL4_LIBS midrender vulkan meshops)
foreach(LIB ${LEVEL4_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( LEVEL5_LIBS  shell )
foreach(LIB ${LEVEL5_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

#-----------
set(wryd_srcs dummy.cpp)

set( wyrd_LIBRARIES
		thirdparty
		${LEVEL0_LIBS}
		${LEVEL1_LIBS}
		${LEVEL2_LIBS}
		${LEVEL3_LIBS}
		${LEVEL4_LIBS}
		${LEVEL5_LIBS}
		)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})
add_library(wyrd_static ${wryd_srcs})

target_link_libraries(wyrd_static ${wyrd_LIBRARIES})
