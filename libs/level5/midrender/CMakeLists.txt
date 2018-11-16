set( MIDRENDER_SRC
		stocks.h
		stocks.cpp
		imguibindings.cpp
		imguibindings.h
		simpleeye.cpp
		simpleeye.h
		meshmodrenderer.cpp
		meshmodrenderer.h
		simpleforwardglobals.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( midrender STATIC ${MIDRENDER_SRC} )
target_link_libraries( midrender imgui)
