set( wyrd_INCLUDES
		${wyrd_INCLUDES}
		${CMAKE_CURRENT_SOURCE_DIR}
		)

set( LEVEL_LIBS
		binny
		math
		input
		)
foreach(LIB ${LEVEL_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)

set( ALL_WYRD_LIBS ${ALL_WYRD_LIBS} ${LEVEL_LIBS} PARENT_SCOPE )
set( wyrd_INCLUDES ${wyrd_INCLUDES} PARENT_SCOPE)
