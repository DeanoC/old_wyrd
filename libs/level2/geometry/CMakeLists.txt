set( GEOMETRY_SRC
		aabb.cpp
		aabb.h
		aabb.inl
		kdtree.cpp
		kdtree.h
		rasteriser.cpp
		rasteriser.h
		ray.cpp
		ray.h
		watertightray.cpp
		watertightray.h
		Frustum.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( geometry STATIC ${GEOMETRY_SRC} )
