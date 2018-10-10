#pragma once
/** \file MeshModScene.h
	A container for a scene hierachy

(c) 2012 Dean Calver
 */

#if !defined( MESH_MOD_SCENE_NODE_H_ )
#define MESH_MOD_SCENE_NODE_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "indextypes.h"
#include "transform.h"
#include "sceneobject.h"
#include "property.h"
#include <functional>

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod {
	class SceneNode;

	typedef std::shared_ptr<SceneNode> SceneNodePtr;
	typedef std::shared_ptr<SceneNode const> ConstSceneNodePtr;

	class SceneNode : public std::enable_shared_from_this<SceneNode> {
	public:
		void addChild( const SceneNodePtr child ) {
			// check for duplicates
			assert( std::find( children.begin(), children.end(), child ) == children.end() );
			children.push_back( child );
		}
		void removeChild( SceneNodePtr child ) {
			SceneNodeContainer::iterator chIt = 
				std::find( children.begin(), children.end(), child );
			assert( chIt != children.end() );
			children.erase( chIt );
		}

		uint32_t findChildIndex( const SceneNodePtr child ) const {
			SceneNodeContainer::const_iterator chIt = std::find( children.begin(), children.end(), child );
			return (uint32_t) std::distance( children.begin(), chIt );
		}

		unsigned int getChildCount() const {
			return (unsigned int) children.size();
		}
		SceneNodePtr getChild( SceneNodeIndex index ) {
			return children[index];
		}
		ConstSceneNodePtr getChild(SceneNodeIndex index) const {
			return children[index];
		}
		void addObject( SceneObjectPtr obj ) {
			assert( std::find( objects.begin(), objects.end(), obj ) == objects.end() );
			objects.push_back( obj );
		}
		void removeObject( SceneObjectPtr obj ) {
			SceneObjectContainer::iterator obIt = 
				std::find( objects.begin(), objects.end(), obj );
			assert( obIt != objects.end() );
			objects.erase( obIt );
		}

		unsigned int getObjectCount() const {
			return (unsigned int) objects.size();
		}
		SceneObjectPtr getObject(unsigned int index) {
			return objects[index];
		}

		void visitDescendents(Math::Matrix4x4 const& rootMatrix, std::function<void(SceneNode const&, Math::Matrix4x4 const&)> func) const;
		void mutateDescendents(Math::Matrix4x4 const& rootMatrix, std::function<void(SceneNode&, Math::Matrix4x4 const&)> func);
		void visitObjects(std::function<void(ConstSceneObjectPtr)> func) const;
		void mutateObjects(std::function<void(SceneObjectPtr)> func);

		// embedded transform TODO animation
		Transform										transform;
		std::string										type;
		std::string										name;
		std::vector< PropertyPtr > 						properties;

	protected:
		typedef std::vector< SceneNodePtr >				SceneNodeContainer;
		typedef std::vector< SceneObjectPtr >			SceneObjectContainer;
		SceneNodeContainer								children;
		SceneObjectContainer							objects;

	};


} // end namespace
#endif // MESH_MOD_SCENE_NODE_H_
