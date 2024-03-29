#include "core/core.h"
#include "meshops/raycaster.h"
#include "meshmod/scene.h"
#include "meshmod/sceneobject.h"
#include "meshmod/vertices.h"
#include "meshmod/halfedges.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata/vertexdata.h"
#include "meshmod/vertexdata/uvvertex.h"
#include "meshmod/vertexdata/positionvertex.h"
#include "meshmod/vertexdata/pointrepvertex.h"
#include "meshmod/halfedgedata/halfedgecontainers.h"
#include "meshmod/polygonsdata/polygoncontainers.h"
#include "geometry/rasteriser.h"
#include "geometry/ray.h"
#include "geometry/kdtree.h"
#include "meshmod/vertexdata/normalvertex.h"

namespace MeshOps {

Raycaster::Raycaster() :
	subSampleCount( 1 ),
	maxDisplacement(std::numeric_limits<float>::infinity()) {

}

void Raycaster::addTargetVertexSource(	const std::string& elementName, 
										const std::string& subElementName, 
										const TRANSFORM_TYPE transformType ) {
	targetVertexSources[ elementName + subElementName ] = transformType;

}
void Raycaster::setTargetMesh(std::shared_ptr<MeshMod::Mesh> _targetMesh, bool doAllVertexSources) {
	using namespace MeshMod;
	if (doAllVertexSources) {
		// iterate through mesh adding all vertex sources
		//todo
	}
	targetMesh = _targetMesh;

	// create a KD-tree from the target mesh
	// need to flatten the position and indices into a simple array for the kdtree

	{
		auto const& vertices = targetMesh->getVertices();
		targetPositionData.resize(size_t(vertices.getCount()) * 3);
		auto tpd = targetPositionData.data();
		vertices.visitAll([tpd, &vertices](VertexIndex vi_)
		{
			size_t const i = size_t(vi_);
			tpd[(i * 3) + 0] = vertices.position(vi_).x;
			tpd[(i * 3) + 1] = vertices.position(vi_).y;
			tpd[(i * 3) + 2] = vertices.position(vi_).z;
		});
	}

	{
		auto const& polygons = targetMesh->getPolygons();
		targetIndexData.resize(size_t(polygons.getCount()) * 3);
		auto tid = targetIndexData.data();
		polygons.visitValidVertices(
			[tid](PolygonIndex pi_, VertexIndex vi_)
			{
				size_t const pi = size_t(pi_);
				size_t const vi = size_t(vi_);
				tid[(pi * 3) + 0] = (unsigned int)(vi);
			});
	}
 
	targetTree = std::make_unique<Geometry::KDTree>(targetPositionData.data(), targetIndexData.data(), (unsigned int) targetIndexData.size());
}

void Raycaster::setBaseMesh(std::shared_ptr<MeshMod::Mesh> _baseMesh, const std::string& uvSetName ) {
	baseMesh = _baseMesh;
	traceUVSetName = uvSetName;
}

void Raycaster::transferTo( LayeredTexture& image ) {
	using namespace Geometry;
	using namespace MeshMod;
	using namespace Math;

	// check the meshes have been set
	// todo change froma assert to user error
	assert( baseMesh );
	assert( targetMesh );

#if 0
	// cache the source data pointers
	if( m_tangentSpaceRequired ) {
		m_tangents.LoadMesh( m_sourceMesh );
		m_binormals.LoadMesh( m_sourceMesh );
	}
#endif

	// locally cache the image dimensions
	const unsigned int  imageWidth = image.getWidth();
	const unsigned int imageHeight = image.getHeight();
	const unsigned int imageSize = imageWidth*imageHeight;

	// create an image channel for displacement
	float* displacement = 0;
	{
		ITextureLayer& dl = image.addLayer<float>( "displacement", 1 );
		displacement = dl.getData<float>();
		memset( displacement, 0, sizeof( float ) * imageSize );
	}

	// create temporary storage for weights
	std::vector<unsigned int> sampleWeights(imageSize);
	memset( sampleWeights.data(), 0, sizeof( unsigned int )*imageSize );

	// TODO multi-thread work load, oriignal used OpenMP should use TBB now
	Rasteriser rasteriser( imageWidth, imageHeight, subSampleCount );

	const PolygonElementsContainer& faceCon = baseMesh->getPolygons().getPolygonsContainer();
	const VerticesElementsContainer& vertCon = baseMesh->getVertices().getVerticesContainer();

	// get base position and normal vertex pointers
	auto const posEle = vertCon.getElement<VertexData::Positions>();
	auto const normEle = vertCon.getElement<VertexData::Normals>();

	// get tracing uv vertex pointer
	auto const uvEle = vertCon.getElement<VertexData::UVs>( traceUVSetName );

	// get face data 
	auto const pfEle = faceCon.getElement<PolygonData::Polygons>();

	auto faceIt = pfEle->elements.begin();
	while( faceIt != pfEle->elements.end() ) {
		const PolygonIndex faceNum = pfEle->distance( faceIt );

		VertexIndexContainer faceVert;
		baseMesh->getPolygons().getVertexIndices(faceNum, faceVert);

		// only works for triangles (TODO handle the others)
		if( faceVert.size() == 3) {	
			// create a triangle with texture coordinate (rasterising in [0,1]x[0,1])
			RasteriserTriangle tri;
			for( unsigned int i = 0; i < 3; ++i ) {
				tri.x[i] = (*uvEle)[ faceVert[i] ].u;
				tri.y[i] = (*uvEle)[ faceVert[i] ].v;
			}
			rasteriser.Reset( &tri );

			// rasterise each pixel within the triangle
			RasteriserPixel pixel;
			while( rasteriser.GetNext( &pixel ) ) {
				// interpolate the position and normal to this point on the triangle
				float coords[] = { float( 1 ) - pixel.v - pixel.w, pixel.v, pixel.w };
				vec3 origin, direction;
				for( unsigned int i = 0; i < 3; ++i ) {
					origin += coords[i] * vec3( ((*posEle)[ faceVert[i] ]).getVec3() );
					direction += coords[i]* vec3( ((*normEle)[ faceVert[i] ]).getVec3() );
				}
				direction = Normalise( direction );

				// construct a ray from these values
				const Ray ray( origin, direction );

				float fMaxDisplacement = maxDisplacement;

#if 0
				// find max distance with hull
				if(!m_hullMesh.IsEmpty()) {
					KDTREE_COLLISION collisionHull;
					if(m_hullTree->IntersectsRay( ray, FLT_MAX, &collisionHull )) {
						fMaxDisplacement = collisionHull.t;
					}
					else {
						std::cerr<<"Warning: No collision with hull"<<std::endl;
					}
				}
#endif
				// raycast into the target mesh
				KDTREE_COLLISION collision;
				if( targetTree->intersectsRay( ray, fMaxDisplacement, &collision ) ) {
#if 0
					int iMaterial=0;
					if(!materialChannel.IsEmpty())
					{
						int* pData = static_cast<int*>(materialChannel.GetData());
						iMaterial = pData[collision.face];
					}
#endif
#if 0
					// get the tangent, binormal and normal for this source ray
					vec3 basis[3];
					if( m_tangentSpaceRequired ) {
						for( int i = 0; i < 3; ++i )
						{
							basis[0] += coords[i]*Vector3( m_tangents.GetVertexData( index + i ) );
							basis[1] += coords[i]*Vector3( m_binormals.GetVertexData( index + i ) );
						}
						basis[2] = direction;
					}
#endif

					// get the pixel location
					unsigned int const offset = ( ( pixel.y*imageWidth ) + pixel.x );

					// update the weight
					++sampleWeights[offset];

					// store the displacement
					displacement[offset] += collision.t;

#if 0
					// sample the attributes
					for( std::vector< RaycasterTargetAttribute >::const_iterator it = m_attributes.begin();
						it != m_attributes.end(); ++it ) {
						RaycasterTargetAttribute const& attr( *it );

						// the uv sample will be used if this attribute is texture sampled
						attr.SampleToImage( pixel, collision, basis, iMaterial);
					}
#endif
				}
			}
		}
	++faceIt;
	}

	float* resultSampleWeights = 0;

	{
		ITextureLayer& rwl = image.addLayer<float>( "weight", 1 );
		resultSampleWeights = rwl.getData<float>();
		memset( resultSampleWeights, 0, sizeof( float ) * imageSize );
	}

	// normalise the results based on the sample weight at each pixel
	for( unsigned int pixel = 0; pixel < imageSize; ++pixel )
	{
		// store the sample weight attribute
		float const weight = float( sampleWeights[pixel] );
		resultSampleWeights[pixel] = weight;

		// normalise the other attributes
		if( sampleWeights[pixel] > 0 )
		{
			float const weight_rcp = float( 1 ) / weight;

			// normalise the displacement
			displacement[pixel] *= weight_rcp;
#if 0
			// normalise the attributes
			for( std::vector< RaycasterTargetAttribute >::const_iterator it = m_attributes.begin();
				it != m_attributes.end(); ++it ) {
				RaycasterTargetAttribute const& attr( *it );

				// get the destination image texel
				mt_float* image = attr.GetImageData( pixel );

				// normalise each component
				for( mt_uint i = 0; i < attr.GetDataComponentCount(); ++i )
					image[i] *= weight_rcp;
			}
#endif
		}
	}
}

}
