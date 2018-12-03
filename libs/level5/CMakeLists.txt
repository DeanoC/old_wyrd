set( wyrd_INCLUDES
		${wyrd_INCLUDES}
		${CMAKE_CURRENT_SOURCE_DIR})

set( LEVEL_LIBS
		tacticalmap
		midrender
		)
foreach(LIB ${LEVEL_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

# shell is added manually to app if the want it but not dlls etc.
add_subdirectory(shell)

set( ALL_WYRD_LIBS ${ALL_WYRD_LIBS} ${LEVEL_LIBS} PARENT_SCOPE )
set( wyrd_INCLUDES ${wyrd_INCLUDES} PARENT_SCOPE)
