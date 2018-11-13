file( GLOB MESHOPS_SRC
		*.cpp
		*.c
		*.h
		*.inl
		VHACD_Lib/public/*.h
		VHACD_Lib/inc/*.h
		VHACD_Lib/src/*.cpp )

add_definitions(${wyrd_DEFINITIONS})
include_directories(
		${wyrd_INCLUDES}
		${CMAKE_CURRENT_SOURCE_DIR}/VHACD_Lib/inc
		)

add_library( meshops STATIC ${MESHOPS_SRC} )
