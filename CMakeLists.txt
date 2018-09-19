cmake_minimum_required(VERSION 3.10)
project(wyrd)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D__STDC_FORMAT_MACROS )
add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE=1 -D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1)

option( BUILD_TESTER   "Build the tester app."                             ON  )
option( BUILD_VIZ      "Build the debug visualiser."                       ON  )


add_subdirectory(enkiTS)
add_subdirectory(binify)
add_subdirectory(tbb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tbb/include)

file( GLOB LZ4_SRC lz4/*.c lz4/*.h)
file( GLOB CRC32C_SRC crc32c/crc32c.cpp crc32c/crc32c.h)
file( GLOB CX_SRC cx/*.h)
file( GLOB CORE_SRC core/*.cpp core/*.c core/*.h core/*.inl)
file( GLOB BINNY_SRC binny/*.cpp binny/*.c binny/*.h binny/*.inl)
file(GLOB RESOURCEMAN_SRC resourceman/*.cpp resourceman/*.c resourceman/*.h resourceman/*.inl)

file( GLOB PLAYFIELD_SRC playfield/*.cpp playfield/*.c playfield/*.h playfield/*.inl)

file( GLOB MESHMOD_SRC  meshmod/*.cpp
                        meshmod/*.c
                        meshmod/*.h
                        meshmod/*.inl
                        meshmod/vertexdata/*.h
                        meshmod/halfedgedata/*.h
                        meshmod/polygonsdata/*.h
                        meshmod/meshdata/*.h
                        meshmod/materialparameterdata/*.h )

file( GLOB MESHOPS_SRC  meshops/*.cpp
                        meshops/*.c
                        meshops/*.h
                        meshops/*.inl
                        meshops/VHACD_Lib/public/*.h
                        meshops/VHACD_Lib/inc/*.h
                        meshops/VHACD_Lib/src/*.cpp )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/meshops/VHACD_Lib/inc)

add_library(wyrd_static ${CORE_SRC}
                        ${LZ4_SRC}
                        ${CRC32C_SRC}
                        ${CX_SRC}
		${BINNY_SRC}
		${RESOURCEMAN_SRC}
		${PLAYFIELD_SRC}
                        ${MESHMOD_SRC}
		${MESHOPS_SRC} resourceman/base.h resourceman/resourceman.cpp resourceman/resourceman.h)

target_link_libraries(wyrd_static enkiTS binify tbb_static tbbmalloc_static tbbmalloc_proxy_static)
set_target_properties(wyrd_static PROPERTIES COMPILE_FLAGS -DCRC32C_STATIC)

if(BUILD_TESTER)
	add_subdirectory(tester)
endif(BUILD_TESTER)
if(BUILD_VIZ)
	add_subdirectory(viz)
endif(BUILD_VIZ)