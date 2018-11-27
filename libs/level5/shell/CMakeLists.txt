set( SHELL_SRC
		interface.h
		consoleapp.cpp
		consoleapp.h
		interface.cpp


)
if (APPLE)
	set(SHELL_SRC ${SHELL_SRC}
			macshell.mm
			macshell.h
			)
elseif(WIN32)
	set(SHELL_SRC ${SHELL_SRC}
			winshell.cpp
			winshell.h
			)
elseif(POSIX)
	set(SHELL_SRC ${SHELL_SRC}
			posixshell.cpp
			posixshell.h
			)
endif ()

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( shell STATIC ${SHELL_SRC} )
if (APPLE)
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	target_link_libraries(shell ${COCOA_LIBRARY})
endif ()