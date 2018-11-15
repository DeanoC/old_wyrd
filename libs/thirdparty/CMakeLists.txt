#--------- Wyrd's third party sources
set( CRC32C_SRC
		crc32c/crc32c.cpp
		crc32c/crc32c.h)

set( CITYHASH_SRC
		cityhash/city.cc
		cityhash/city.h
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


add_definitions(${wyrd_DEFINITIONS})
include_directories(${wyrd_INCLUDES})

set( THIRDPARTY_LIBS enkiTS shaderc)
foreach(LIB ${THIRDPARTY_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)
add_subdirectory(tbb)
set( THIRDPARTY_LIBS ${THIRDPARTY_LIBS} tbb_static tbbmalloc_static)



add_library( thirdparty STATIC
		${CRC32C_SRC}
		${CITYHASH_SRC}
		${FMT_SRC}
		${LZ4_SRC}
		${IMGUI_SRC}
		)
target_link_libraries(thirdparty ${THIRDPARTY_LIBS})
