cmake_minimum_required(VERSION 3.9)
project(tester)

set(CMAKE_CXX_STANDARD 17)
set(TESTER_SOURCE
		core/math_unittest.cpp
		core/freelist_unittest.cpp
		resourcemanager/resourcemanager_unittest.cpp
		tester.cpp
		)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(tester WIN32 ${TESTER_SOURCE})
add_definitions(-DUSING_STATIC_LIBS)
target_link_libraries(tester wyrd_static binify)
target_compile_definitions(tester PRIVATE
		"-D_CRT_SECURE_NO_WARNINGS" "-D__STDC_FORMAT_MACROS")

