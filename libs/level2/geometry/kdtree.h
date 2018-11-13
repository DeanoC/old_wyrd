#pragma once
#if !defined( WYRD_GEOMETRY_KDTREE_H )
#define WYRD_GEOMETRY_KDTREE_H

#include <memory>
#include <vector>
#include "geometry/aabb.h"

namespace Geometry {

class Ray;
class AABB;

//! Node in a k-d tree.
class KDTreeNode {
public:
	//! Creates an empty node.
	KDTreeNode();
	~KDTreeNode();

	//! Sets this node as a parent.
	void set( int axis, float coordinate );

	//! Sets this node as a leaf.
	void setLeaf( unsigned int faceCount );

	//! Returns true if this node is a leaf.
	bool isLeaf() const { return m_faceCount != 0; }

	//! Gets the children of a parent node.
	KDTreeNode* getChild( int index ) const { 
		assert( !isLeaf() );
		assert( 0 <= index && index <= 1 );
		return m_children[index];
	}

	//! Gets the axis index of a parent node.
	int getAxis() const { assert( !isLeaf() ); return m_axis; }

	//! Gets the axis coordinate of a parent node.
	float getCoordinate() const { assert( !isLeaf() ); return m_coordinate; }

	//! Gets the faces in a leaf node.
	unsigned int const* getFaces() const { assert( isLeaf() ); return m_faces.data(); }

	//! Gets the faces in a leaf node.
	unsigned int * getFaces() { assert( isLeaf() ); return m_faces.data(); }

	//! Gets the number of faces in a leaf node.
	unsigned int getFaceCount() const { assert( isLeaf() ); return m_faceCount; }

private:
	KDTreeNode* m_children[2];
	int m_axis;
	float m_coordinate;

	std::vector<unsigned int> m_faces;
	unsigned int m_faceCount;
};

struct KDTREE_COLLISION{
	unsigned int face;
	float v, w;
	float t;
};

//! A simple k-d tree.
class KDTree {
public:
//	explicit KDTree( meshtools::Mesh const& mesh );
	explicit KDTree( const float* positionData, const unsigned int* indexData, const unsigned int indexCount );

	bool intersectsRay( Ray const& ray, float maxRange, KDTREE_COLLISION* collision ) const;

private:
	typedef std::vector< unsigned int > FaceArray;

	void buildSubtree( KDTreeNode* node, 
		FaceArray& faces, 
		AABB const& bounds );

	void partition( FaceArray const& faces, 
		AABB& bounds, 
		FaceArray& partition ) const;

	float const* m_data;
	unsigned int const* m_indices;

	std::unique_ptr<KDTreeNode> m_root;
	AABB m_bounds;

	int m_currentDepth;
	int m_leafDepthCount;
	int m_leafCount;
	unsigned int m_leafTriangleCount;
};

} // end namespace Core
#endif