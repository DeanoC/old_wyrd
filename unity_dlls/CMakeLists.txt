set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../Assets/Plugins)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library(CUtils SHARED cutils.cpp cutils.h)
set_target_properties(CUtils PROPERTIES COMPILE_FLAGS -DCRC32C_STATIC)
target_link_libraries(CUtils wyrd_static)
set_target_properties(CUtils PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		)
set_target_properties(CUtils PROPERTIES SUFFIX ".nco")

add_library(CGeometryEngine SHARED cgeometryengine.cpp )
target_link_libraries(CGeometryEngine wyrd_static CUtils)

set_target_properties(CGeometryEngine PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		)
set_target_properties(CGeometryEngine PROPERTIES SUFFIX ".nco")

add_library(CTacticalMap SHARED ctacticalmap.cpp)
target_link_libraries(CTacticalMap wyrd_static CGeometryEngine CUtils)

set_target_properties(CTacticalMap PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		)
set_target_properties(CTacticalMap PROPERTIES SUFFIX ".nco")

add_library(ncoproxy SHARED ncoproxy.cpp)
set_target_properties(ncoproxy PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_SOURCE_DIR}/../../Assets/Plugins"
		)

