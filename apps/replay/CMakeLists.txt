cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(REPLAYAPP_SOURCE
		replayapp.cpp
		server.cpp
		server.h
		fakeclient.cpp
		fakeclient.h
		../appcommon/arcballcamera.h
		../appcommon/simplepadcamera.h)
include_directories( ${apps_INCLUDES} ${wyrd_INCLUDES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/live)
add_executable(replayapp WIN32 ${REPLAYAPP_SOURCE} )
target_link_libraries(replayapp wyrd_static shell)#ncoproxy CTacticalMap CGeometryEngine CUtils)
target_compile_definitions(replayapp PRIVATE ${wyrd_DEFINITIONS} )

