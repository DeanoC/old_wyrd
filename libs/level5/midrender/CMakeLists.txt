set( MIDRENDER_SRC
		stocks.h
		stocks.cpp
		imguibindings.cpp imguibindings.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( midrender STATIC ${MIDRENDER_SRC} )
