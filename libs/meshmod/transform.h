#pragma once
/** \file MeshModTransform.h
	Holds a transform for a scene node
   (c) 2006 Dean Calver
 */

#if !defined( MESH_MOD_TRANSFORM_H_ )
#define MESH_MOD_TRANSFORM_H_

#include "math/vector_math.h"

namespace MeshMod
{

//! an object holding 
class Transform
{
public:
	Transform() :
			position(0,0,0),
			orientation( 0,0,0,1 ),
			scale( 1,1,1 ) 
	{}
	bool isIdentity() const {
		if ( (Math::LengthSquared( position ) < 1e-5f) &&
			 (1 - Math::LengthSquared(orientation) < 1e-5f) &&
			 (3 - Math::LengthSquared( scale ) < 1e-5f) ) {
			return true;
		} else {
			return false;
		}
	}

	// just one of many options for this...
	Math::Matrix4x4 MakeMatrix() const
	{
		Math::Matrix4x4 transMat = Math::CreateTranslationMatrix(position);
		Math::Matrix4x4 rotMat = Math::CreateRotationMatrix(orientation);
		Math::Matrix4x4 scaleMat = Math::CreateScaleMatrix(scale);

		return scaleMat * rotMat * transMat;
	}

	//! position
	Math::Vector3		position;
	//! orientation
	Math::Quaternion	orientation;
	//! scale
	Math::Vector3		scale;
};

};

#endif // end MESH_MOD_TRANSFORM_H_