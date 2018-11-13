set( TACTICALMAP_SRC
		tacticalmap.cpp
		builder.cpp
		stitcher.cpp
		tacticalmap.h
		builder.h
		stitcher.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( tacticalmap STATIC ${TACTICALMAP_SRC} )
