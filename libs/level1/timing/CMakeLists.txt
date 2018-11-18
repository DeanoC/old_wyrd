set( TIMING_SRC
		tickerclock.cpp
		tickerclock.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( timing STATIC ${TIMING_SRC} )