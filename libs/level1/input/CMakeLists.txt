set( INPUT_SRC
		input.h
		keylist.h
		keyboard.h
		mouse.h
		vpadlistener.h
		provider.h
		keyboardmouselistenerimpl.cpp
		keyboardmouselistenerimpl.h
		)
if (APPLE)
	set(INPUT_SRC ${INPUT_SRC}
			osx.mm
			osx_keycodes.h
			)
elseif(WIN32)
	set(INPUT_SRC ${INPUT_SRC}
			windows.cpp
			)
elseif(POSIX)
	set(INPUT_SRC ${INPUT_SRC}
			)
endif ()
add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( input STATIC ${INPUT_SRC} )