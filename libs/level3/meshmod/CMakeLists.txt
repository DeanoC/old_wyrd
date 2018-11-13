file( GLOB MESHMOD_SRC
		*.cpp
		*.c
		*.h
		*.inl
		vertexdata/*.h
		halfedgedata/*.h
		polygonsdata/*.h
		meshdata/*.h
		materialparameterdata/*.h )

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( meshmod STATIC ${MESHMOD_SRC} )
