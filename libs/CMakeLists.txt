
#--------- Wyrd's third party sources
find_package(vulkan REQUIRED)

option(ENKITS_BUILD_C_INTERFACE "" OFF )
option(ENKITS_BUILD_EXAMPLES "" OFF )
option(TBB_BUILD_SHARED "" OFF)
option(TBB_BUILD_STATIC "" ON)
option(TBB_BUILD_TBBMALLOC "" ON)
option(TBB_BUILD_TBBMALLOC_PROXY "" OFF)
option(TBB_BUILD_TESTS "" OFF)

add_subdirectory(enkiTS)
add_subdirectory(binify)
add_subdirectory(tbb)

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

#------------ Wyrd's own sources ------------
set( BINNY_SRC
		binny/bundle.cpp
		binny/bundle.h
		binny/bundlewriter.cpp
		binny/bundlewriter.h
		binny/ibundle.h
		binny/inmembundle.h
		binny/writehelper.cpp
		binny/writehelper.h
		)

set( CORE_SRC
		core/any.h
		core/core.h
		core/exception.h
		core/filesystem.h
		core/freelist.h
		core/linear_allocator.h
		core/loguru.hpp
		core/platform.h
		core/platform_cuda.h
		core/platform_linux.h
		core/platform_osx.h
		core/platform_posix.h
		core/platform_win.h
		core/quick_hash.h
		core/utils.h
		)
set( GEOMETRY_SRC
		geometry/aabb.cpp
		geometry/aabb.h
		geometry/aabb.inl
		geometry/kdtree.cpp
		geometry/kdtree.h
		geometry/rasteriser.cpp
		geometry/rasteriser.h
		geometry/ray.cpp
		geometry/ray.h
		geometry/watertightray.cpp
		geometry/watertightray.h
		)

set( RENDER_SRC
		render/generictextureformat.h
		render/gtfcracker.h
		render/stable.h
		render/device.h
		render/display.h
		render/texture.cpp
		render/texture.h
		render/image.inl
		render/image_fetch.inl
		render/image_put.inl
		render/image.h
		render/resources.cpp render/resources.h render/commandqueue.h render/encoder.h render/renderpass.h render/rendertarget.h render/semaphore.h render/fence.h render/types.h render/renderpass.cpp render/image.cpp render/rendertarget.cpp)

set( MATH_SRC
		math/vector_math.cpp
		math/vector_math.h
		math/vector_math_c.h
		math/colourspace.h)

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

set( RESOURCEMAN_SRC
		resourcemanager/resource.h
		resourcemanager/diskstorage.h
		resourcemanager/istorage.h
		resourcemanager/memstorage.h
		resourcemanager/resourcecache.cpp
		resourcemanager/resourcecache.h
		resourcemanager/resourceman.cpp
		resourcemanager/resourceman.h
		resourcemanager/resourcename.h
		resourcemanager/textresource.h
		resourcemanager/textresource.cpp resourcemanager/writer.h resourcemanager/resourcehandle.h resourcemanager/resourceid.h)

set( SHELL_SRC
		shell/interface.h
		shell/consoleapp.cpp
		shell/consoleapp.h
		shell/interface.cpp
		shell/posixshell.cpp
		shell/posixshell.h
		shell/winshell.cpp
		shell/winshell.h)

set( VULKAN_SRC
		vulkan/vkfcracker.h
		vulkan/system.h
		vulkan/system.cpp
		vulkan/functionlist.inl
		vulkan/api.cpp
		vulkan/api.h
		vulkan/display.h vulkan/device.cpp vulkan/device.h vulkan/vk_mem_alloc.h vulkan/vk_mem_alloc.cpp vulkan/texture.cpp vulkan/texture.h vulkan/display.cpp vulkan/commandqueue.cpp vulkan/commandqueue.h vulkan/encoder.cpp vulkan/encoder.h vulkan/rendertarget.cpp vulkan/rendertarget.h vulkan/fence.cpp vulkan/fence.h vulkan/renderpass.cpp vulkan/renderpass.h shell/posixshell.cpp shell/posixshell.h vulkan/encoderpool.cpp vulkan/encoderpool.h vulkan/renderencoder.cpp vulkan/renderencoder.h vulkan/computeencoder.cpp vulkan/computeencoder.h vulkan/types.h vulkan/resources.cpp vulkan/resources.h)

#-----------
set(wryd_srcs
		${CITYHASH_SRC}
		${CRC32C_SRC}
		${CX_SRC}
		${FMT_SRC}
		${LZ4_SRC}

		${CORE_SRC}
		${BINNY_SRC}
		${MATH_SRC}
		${MESHMOD_SRC}
		${MESHOPS_SRC}
		${GEOMETRY_SRC}
		${RENDER_SRC}
		${RESOURCEMAN_SRC}
		${SHELL_SRC}
		${VULKAN_SRC}
		vulkan/semaphore.cpp vulkan/semaphore.h)
set( wyrd_LIBRARIES enkiTS
					binify
					tbb_static
					tbbmalloc_static
					)
list(APPEND wyrd_LIBRARIES ${glfw_LIBRARIES})

set( wyrd_DEFINITIONS
	-D__TBB_NO_IMPLICIT_LINKAGE=1 
	-D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1
	-D_CRT_SECURE_NO_WARNINGS -D__STDC_FORMAT_MACROS
	-DCRC32C_STATIC
	)
set( wyrd_DEFINITIONS ${wyrd_DEFINITIONS} PARENT_SCOPE )

set( wyrd_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/tbb/include
	${CMAKE_CURRENT_SOURCE_DIR}/fmt/include
	${Vulkan_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/meshops/VHACD_Lib/inc
	)
set( wyrd_INCLUDES ${wyrd_INCLUDES} PARENT_SCOPE )

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})
add_library(wyrd_static ${wryd_srcs})

target_link_libraries(wyrd_static ${wyrd_LIBRARIES})
