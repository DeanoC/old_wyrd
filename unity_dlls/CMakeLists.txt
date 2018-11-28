
add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library(CUtils SHARED cutils.cpp cutils.h)
set_target_properties(CUtils PROPERTIES
		COMPILE_FLAGS -DCRC32C_STATIC
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		SUFFIX ".nco"
		DEBUG_POSTFIX ""
		)
target_link_libraries(CUtils wyrd_static)

add_library(CGeometryEngine SHARED cgeometryengine.cpp )
target_link_libraries(CGeometryEngine wyrd_static CUtils)

set_target_properties(CGeometryEngine PROPERTIES
		COMPILE_FLAGS -DCRC32C_STATIC
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		SUFFIX ".nco"
		DEBUG_POSTFIX ""
		)

add_library(CTacticalMap SHARED ctacticalmap.cpp)
target_link_libraries(CTacticalMap wyrd_static CGeometryEngine CUtils)

set_target_properties(CTacticalMap PROPERTIES
		COMPILE_FLAGS -DCRC32C_STATIC
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		SUFFIX ".nco"
		DEBUG_POSTFIX ""
		)

add_library(ncoproxy SHARED ncoproxy.cpp)
set_target_properties(ncoproxy PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		DEBUG_POSTFIX ""
		)

