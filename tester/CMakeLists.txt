cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(TESTER_SOURCE
		core/freelist_unittest.cpp
		resourcemanager/resourcemanager_unittest.cpp
		tester.cpp
		render/generictextureformat_unittest.cpp
		vulkan/system_unittest.cpp binny/bundle_unittest.cpp math/scalar_math_unittest.cpp render/image_unittest.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/live)
add_executable(tester WIN32 ${TESTER_SOURCE})
add_definitions(-DUSING_STATIC_LIBS)
target_link_libraries(tester wyrd_static)
include_directories( ${wyrd_INCLUDES})
target_compile_definitions(tester PRIVATE ${wyrd_DEFINITIONS})

