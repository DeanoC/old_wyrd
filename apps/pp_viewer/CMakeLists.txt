cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(PP_VIEWER_SOURCE
		pp_viewer.cpp
		server.cpp
		server.h)
include_directories( ${wyrd_INCLUDES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/live)
add_executable(pp_viewer WIN32 ${PP_VIEWER_SOURCE} )
target_link_libraries(pp_viewer wyrd_static ncoproxy CTacticalMap CGeometryEngine CUtils)
target_compile_definitions(pp_viewer PRIVATE ${wyrd_DEFINITIONS} )

