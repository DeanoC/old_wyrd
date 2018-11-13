set( MATH_SRC
		vector_math.cpp
		vector_math.h
		vector_math_c.h
		colourspace.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( math STATIC ${MATH_SRC} )
