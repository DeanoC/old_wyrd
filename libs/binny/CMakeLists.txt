set( BINNY_SRC
		bundle.cpp
		bundle.h
		bundlewriter.cpp
		bundlewriter.h
		ibundle.h
		inmembundle.h
		writehelper.cpp
		writehelper.h
		)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( binny STATIC ${BINNY_SRC} )
