set( SHELL_SRC
		interface.h
		consoleapp.cpp
		consoleapp.h
		interface.cpp
		posixshell.cpp
		posixshell.h
		winshell.cpp
		winshell.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( shell STATIC ${SHELL_SRC} )
