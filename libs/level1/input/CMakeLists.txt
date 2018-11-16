set( INPUT_SRC
		input.h
		keylist.h
		keyboard.h
		windows.cpp
		mouse.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( input STATIC ${INPUT_SRC} )