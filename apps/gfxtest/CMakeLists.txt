cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(GFXTEST_SOURCE
	gfxtest.cpp
		basicmeshtest.cpp gfxtest.h)
include_directories( ${wyrd_INCLUDES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/live)
add_executable(gfxtest WIN32 ${GFXTEST_SOURCE})
add_definitions(-DUSING_STATIC_LIBS)
target_link_libraries(gfxtest wyrd_static shell)
target_compile_definitions(gfxtest PRIVATE ${wyrd_DEFINITIONS} )

