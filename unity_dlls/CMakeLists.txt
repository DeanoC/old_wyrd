cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D__STDC_FORMAT_MACROS )
add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE=1 -D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1)

add_library(CUtils SHARED cutils.cpp cutils.h)
set_target_properties(CUtils PROPERTIES COMPILE_FLAGS -DCRC32C_STATIC)
target_link_libraries(CUtils wyrd_static)
set_target_properties(CUtils PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		)
set_target_properties(CUtils PROPERTIES SUFFIX ".nco")

add_library(CGeometryEngine SHARED cgeometryengine.cpp)
target_link_libraries(CGeometryEngine wyrd_static)
set_target_properties(CGeometryEngine PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		)
set_target_properties(CGeometryEngine PROPERTIES SUFFIX ".nco")



add_library(CTacticalMap SHARED ${TACTICALMAP_SRC} ctacticalmap.cpp)
target_link_libraries(CTacticalMap wyrd_static CGeometryEngine CUtils)

set_target_properties(CTacticalMap PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		)
set_target_properties(CTacticalMap PROPERTIES SUFFIX ".nco")

add_library(ncoproxy SHARED ncoproxy.cpp)
set_target_properties(ncoproxy PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Plugins"
		)

