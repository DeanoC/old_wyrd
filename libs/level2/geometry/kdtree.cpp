///-------------------------------------------------------------------------------------------------
/// \file	core\kdtree.cpp
///
/// \brief	Implements the kdtree class.
///
/// \details	
///		kdtree description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. Public Domain.
/// \remark	mailto://deano@cloudpixies.com
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <memory>
#include "core/core.h"
#include "math/vector_math.h"
#include "geometry/ray.h"
#include "geometry/aabb.h"
#include "geometry/kdtree.h"
#include <array>

using namespace Math;

namespace Geometry {

static int const g_maxTreeDepth = 16;
static unsigned int const g_minFacesPerLeaf = 64;

KDTreeNode::KDTreeNode()
	: m_axis( 0 ), 
	m_coordinate( 0 ), 
	m_faceCount( 0 )
{
	m_children[0] = nullptr;
	m_children[1] = nullptr;
}
KDTreeNode::~KDTreeNode()
{
	delete m_children[0];
	delete m_children[1];
}

void KDTreeNode::set( int axis, float coordinate ) {
	assert( axis >= 0 );
	m_axis = axis;
	m_coordinate = coordinate;

	m_faceCount = 0;
	m_faces.clear();

	// optimize these using in place or a pool
	m_children[0] = new KDTreeNode();
	m_children[1] = new KDTreeNode();
}

void KDTreeNode::setLeaf( unsigned int faceCount ) {
	assert( faceCount > 0 );
	m_faceCount = faceCount;
	m_faces.resize(faceCount);

	m_axis = 0;
	m_coordinate = float( 0 );
}

KDTree::KDTree( const float* positionData, const unsigned int* indexData, const unsigned int indexCount ) :
	m_data( positionData ), 
	m_indices( indexData ), 
	m_currentDepth( 0 ), 
	m_leafDepthCount( 0 ), 
	m_leafCount( 0 ), 
	m_leafTriangleCount( 0 ) {

	// get the bounds of the mesh
	for( unsigned int i = 0; i < indexCount; i+=3 ) {
		Math::vec3 a = Math::Vec3FromArray( positionData+ (indexData[i+0] * 3) );
		Math::vec3 b = Math::Vec3FromArray( positionData+ (indexData[i+1] * 3) );
		Math::vec3 c = Math::Vec3FromArray( positionData+ (indexData[i+2] * 3) );
		m_bounds.expandBy( a );
		m_bounds.expandBy( b );
		m_bounds.expandBy( c );
	}

	// create a list of polygons (triangles at the moment)
	unsigned int const triangleCount = indexCount / 3;
	FaceArray faces;
	faces.reserve( triangleCount );
	for( unsigned int face = 0; face < triangleCount; ++face )
		faces.push_back( face );

	// build the tree
	m_root = std::make_unique<KDTreeNode>();
	buildSubtree( m_root.get(), faces, m_bounds );
}

void KDTree::buildSubtree( KDTreeNode* node, FaceArray& faces, AABB const& bounds ) {
	// advance the depth
	++m_currentDepth;

	// fill in the node as either a leaf or parent
	if( faces.size() < g_minFacesPerLeaf || m_currentDepth == g_maxTreeDepth ) {
		// push statistics
		m_leafTriangleCount += (unsigned int) faces.size();
		m_leafDepthCount += m_currentDepth;
		++m_leafCount;

		// leaf node
		node->setLeaf( (unsigned int)faces.size() );
		memcpy( node->getFaces(), &faces[0], faces.size()*sizeof( unsigned int ) );

		// clear up our data
		faces.clear();
	} else {
		// split along the largest AABB axis
		vec3 length = bounds.getMaxExtent() - bounds.getMinExtent();
		int axis;
		if( length.x > length.y )
			axis = ( length.x > length.z ) ? 0 : 2;
		else
			axis = ( length.y > length.z ) ? 1 : 2;

		float coordinate = bounds.getBoxCenter()[axis];

		// create a parent node
		node->set( axis, coordinate );

		// batch up the polygons "under" the boundary
		vec3 boundsMin = bounds.getMaxExtent();
		vec3 boundsMax = bounds.getMinExtent();

		vec3 underMax = boundsMax;
		underMax[axis] = coordinate;
		AABB underBounds( boundsMin, underMax );

		vec3 overMin = boundsMin;
		overMin[axis] = coordinate;
		AABB overBounds( overMin, boundsMax );

		FaceArray underFaces, overFaces;
		partition( faces, underBounds, underFaces );
		partition( faces, overBounds, overFaces );

		// release memory we don't need any more
		faces.clear();

		// create the subtrees
		buildSubtree( node->getChild( 0 ), underFaces, underBounds );
		buildSubtree( node->getChild( 1 ), overFaces, overBounds );
	}

	// reduce the depth
	--m_currentDepth;
}

void KDTree::partition( FaceArray const& faces, AABB& bounds, FaceArray& partition ) const {
	assert( !faces.empty() );

	// store and bound all the polygons within the given bounds
	AABB partitionBounds;
	bool passed = false;
	for( FaceArray::const_iterator it = faces.begin();
		it != faces.end(); ++it ) {
		// get the triangle vertex positions
		unsigned int const face = *it;
		std::array<vec3,3> points {
			Vec3FromArray( m_data + 3*m_indices[3*face]  ),
			Vec3FromArray( m_data + 3*m_indices[3*face + 1] ),
		    Vec3FromArray( m_data + 3*m_indices[3*face + 2] )
		};

		// compute the local bounds
		AABB localBounds(
			min( points[0], min( points[1], points[2] ) ),
			max( points[0], max( points[1], points[2] ) )
			);

		// pass this face if it intersect the bounds
		if( localBounds.intersects( bounds ) ) {
			partition.push_back( face );
			if( passed ) {	
				partitionBounds.unionWith( localBounds );
			} else {
				partitionBounds = localBounds;
				passed = true;
			}
		}
	}

	// clip the partition bounds to the parent ones
	if( passed )
		bounds.intersectWith( partitionBounds );
}

bool KDTree::intersectsRay( Ray const& ray, float maxRange, KDTREE_COLLISION* collision ) const {
	float current = -1;

	// bound the line segment from the root bounds
	float min, max;
	if( !ray.intersectsAABB( m_bounds, min, max ) )
		return false;

	// set up the stack
	KDTreeNode const* node = m_root.get();
	struct STACK_ELEMENT {
		KDTreeNode const* node;	//!< The next node.
		float min, max;			//!< The segment min and max for the node.
	};
	// create the stack for ray intersection tests
	STACK_ELEMENT stack[g_maxTreeDepth];

	int nextStackIndex = 0;

	// precompute some stuff
	Math::vec3 direction_rcp = Math::Reciprocal(ray.getDirection());

	// recurse
	bool hit = false;
	for( ;; ) {
		// assume we have a valid current node, process it
		for( ;; ) {
			if( node->isLeaf() ) {
				// check every triangle in this bin for a hit closer than maxRange
				unsigned int const* faces = node->getFaces();
				unsigned int const faceCount = node->getFaceCount();
				for( unsigned int i = 0; i < faceCount; ++i ) {
					unsigned int const i0 = (3*faces[i]) + 0;
					unsigned int const i1 = (3*faces[i]) + 1;
					unsigned int const i2 = (3*faces[i]) + 2;

					// test this triangle against the ray
					float v, w, t;
					const vec3 v0 = Vec3FromArray( m_data + 3*m_indices[i0] );
					const vec3 v1 = Vec3FromArray( m_data + 3*m_indices[i1] );
					const vec3 v2 = Vec3FromArray( m_data + 3*m_indices[i2] );
					if( ray.intersectsTriangle( v0, v1, v2, v, w, t ) ) {
						//mt_float range = std::abs( t );
						float range = t;
						if( (current < range) && (range < maxRange) ) {
							// record the hit and distance
							hit = true;
							current = t;

							// record the triangle and coordinate on it
							collision->face = faces[i];
							collision->v = v;
							collision->w = w;

							// record the ray position
							collision->t = t;
						}
					}
				}

				// break out of this loop to start popping the stack
				break;
			} else {
				// let the ray hit the splitting plane
				int const axis = node->getAxis();
				float const coord = ( node->getCoordinate() - ray.getOrigin()[axis] )*direction_rcp[axis];
				int furthestChild = ( ray.getDirection()[axis] > float( 0 ) ) ? 1 : 0;

				// check this test got reasonable results
				if( std::isfinite( coord ) ) {
					// recurse based on where the coordinate lies in relation to our min and max
					if( coord < min ) {
						// if we missed the near child, we must have hit the furthest one
						node = node->getChild( furthestChild );
					} else if( coord > max ) {
						// if we missed the furthest child, we must have hit the near one
						node = node->getChild( 1 - furthestChild );
					} else {
						assert( nextStackIndex != g_maxTreeDepth );
						if( coord < float( 0 ) ) {
							// otherwise we hit both children, so push the furthest node onto the stack
							stack[nextStackIndex].node = node->getChild( 1 - furthestChild );
							stack[nextStackIndex].min = min;
							stack[nextStackIndex].max = coord;
							++nextStackIndex;

							// and recurse to the near one
							node = node->getChild( furthestChild );
							min = coord;
						} else {
							// otherwise we hit both children, so push the furthest node onto the stack
							stack[nextStackIndex].node = node->getChild( furthestChild );
							stack[nextStackIndex].min = coord;
							stack[nextStackIndex].max = max;
							++nextStackIndex;

							// and recurse to the near one
							node = node->getChild( 1 - furthestChild );
							max = coord;
						}
					}
				} else {
					// recurse based on which side of the split our ray lies in
					node = node->getChild( ( ray.getOrigin()[axis] > node->getCoordinate() ) ? 1 : 0 );
				}
			}
		}

		// we ran out of nodes, so pop one off the stack
		for( ;; ) {
			// attempt to pop an item the stack, if we can't we're done!
			if( --nextStackIndex < 0 )
				return hit;

			// check this item is valid, if so then use it
			if( stack[nextStackIndex].max > -maxRange && stack[nextStackIndex].min < maxRange ) {
				node = stack[nextStackIndex].node;
				min = stack[nextStackIndex].min;
				max = stack[nextStackIndex].max;
				break;
			}
		}
	}
}

}