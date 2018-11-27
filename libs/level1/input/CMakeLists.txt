set( INPUT_SRC
		input.h
		keylist.h
		keyboard.h
		windows.cpp
		mouse.h
		vpadlistener.h
		provider.h keyboardmouselistenerimpl.cpp keyboardmouselistenerimpl.h osx.mm osx_keycodes.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( input STATIC ${INPUT_SRC} )