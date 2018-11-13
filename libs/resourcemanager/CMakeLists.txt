set( RESOURCEMAN_SRC
		resource.h
		diskstorage.h
		istorage.h
		memstorage.h
		resourcecache.cpp
		resourcecache.h
		resourceman.cpp
		resourceman.h
		resourcename.h
		textresource.h
		textresource.cpp
		writer.h
		resourcehandle.h
		resourceid.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( resourcemanager STATIC ${RESOURCEMAN_SRC} )
