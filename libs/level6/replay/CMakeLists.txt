set( REPLAY_SRC
		replay.cpp
		replay.h
		gui.cpp
		gui.h
		items.cpp
		items.h
		propertyeditor.h
)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( replay STATIC ${REPLAY_SRC} )
target_link_libraries( replay midrender)
