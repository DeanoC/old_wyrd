file( GLOB MESHOPS_SRC  meshops/*.cpp
		*.c
		*.h
		*.inl
		VHACD_Lib/public/*.h
		VHACD_Lib/inc/*.h
		VHACD_Lib/src/*.cpp )

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( meshops STATIC ${MESHOPS_SRC} )
