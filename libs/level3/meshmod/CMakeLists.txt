set(MESHMOD_SRC
		genericcontainters.h
		halfedges.cpp
		halfedges.h
		indextypes.h
		mesh.cpp
		mesh.h
		meshmod.h
		polygons.cpp
		polygons.h
		property.h
		scene.h
		scenenode.cpp
		scenenode.h
		sceneobject.h
		transform.h
		triangle.h
		types.h
		varicontainer.cpp
		varicontainer.h
		varielements.h
		vertices.cpp
		vertices.h
		vertexdata/colourvertex.h
		vertexdata/normalvertex.h
		vertexdata/pointrepvertex.h
		vertexdata/positionvertex.h
		vertexdata/uvvertex.h
		vertexdata/vertexcontainers.h
		vertexdata/vertexdata.h
		halfedgedata/halfedgecontainers.h
		halfedgedata/halfedgedata.h
		polygonsdata/polygoncontainers.h
		polygonsdata/polygondata.h
		meshdata/materialcontainers.h
		meshdata/materialdata.h
		materialparameterdata/materialparameter.h
		materialparameterdata/materialparameterbase.h
		materialparameterdata/materialvertexbindings.h
		materialparameterdata/rgbmaterialparameter.h
		materialparameterdata/scalarmaterialparameter.h
		materialparameterdata/texturematerialparameter.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( meshmod STATIC ${MESHMOD_SRC} )
