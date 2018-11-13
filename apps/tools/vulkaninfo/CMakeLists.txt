cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(VULKANINFO_SOURCE
		vulkaninfo.cpp
		)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/live)
add_executable(vulkaninfo WIN32 ${VULKANINFO_SOURCE})
add_definitions(-DUSING_STATIC_LIBS)
target_link_libraries(vulkaninfo wyrd_static)
include_directories(${wyrd_INCLUDES})
target_compile_definitions(vulkaninfo PRIVATE ${wyrd_DEFINITIONS})

