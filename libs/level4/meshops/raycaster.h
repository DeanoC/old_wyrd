#pragma once
#if !defined( MESHOPS_RAYCASTER_H_ )
#define MESHOPS_RAYCASTER_H_

#include "geometry/kdtree.h"
#include "meshops/layeredtexture.h"

namespace MeshMod {
class Mesh;
}


namespace MeshOps {

//! The type of transform to apply to an element during raycasting.
enum TRANSFORM_TYPE {
	/*! Do not apply a transform i.e. raycast in object space. */
	TRANSFORM_DEFAULT = 0,

	/*! Rotate the sample into the tangent space of the source mesh.
		This requires the target element to have 3 components and requires 
		the presence of the tangent space basis on the source mesh. */
	TRANSFORM_TANGENTSPACE, 
};

///-------------------------------------------------------------------------------------------------
/// \class	Raycaster
///
/// \brief	Raycaster transfer elements from the target mesh onto the base mesh
///
/// \details Trasfers elements and textures from the target mesh onto the base mesh texture. 
/// 		 Can convert things like normal map space, generate displacement maps etc.
/// \todo vertex element transfers as well
////////////////////////////////////////////////////////////////////////////////////////////////////
class Raycaster {
public:
	Raycaster();
	// target (where we will transfer elements FROM) mesh API
	///-------------------------------------------------------------------------------------------------
	/// \fn
	/// void Raycaster::addTargetVertexSource( const Core::string& elementName,
	/// const Core::string& subElementName = "",
	/// const TRANSFORM_TYPE transfromType = TRANSFORM_DEFAULT );
	///
	/// \brief	Adds a specific target vertex source.
	///
	/// \param	elementName   	Name of the element. 
	/// \param	subElementName	(optional) name of the sub element. 
	/// \param	transfromType 	(optional) type of the transfrom. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void addTargetVertexSource(	const std::string& elementName, 
								const std::string& subElementName = "", 
								const TRANSFORM_TYPE transfromType = TRANSFORM_DEFAULT );

	///-------------------------------------------------------------------------------------------------
	/// \fn
	/// void Raycaster::setTargetMesh( const MeshMod::MeshPtr _targetMesh,
	/// bool doAllVertexSources = true );
	///
	/// \brief	Sets a transfer mesh.
	///
	/// \todo	Fill in detailed method description.
	///
	/// \param	_targetMesh		  	The transfer mesh. 
	/// \param	doAllVertexSources	(optional) do all target mesh vertex sources. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void setTargetMesh(std::shared_ptr<MeshMod::Mesh> _targetMesh, bool doAllVertexSources = true);

	// base mesh API
	void setBaseMesh(std::shared_ptr<MeshMod::Mesh> _baseMesh, std::string const& uvSetName);
	void setMaxDisplacement( float _maxDisplacement ) { maxDisplacement = _maxDisplacement; }
	void setSubSampleCount( unsigned int _subSampleCount ) { subSampleCount = _subSampleCount; }

	void transferTo( LayeredTexture& image );
private:
	std::map<std::string, TRANSFORM_TYPE> targetVertexSources;
	std::shared_ptr<MeshMod::Mesh> targetMesh;
	std::shared_ptr<MeshMod::Mesh> baseMesh;
	std::string			traceUVSetName;
	unsigned int		subSampleCount;
	float				maxDisplacement;
	std::unique_ptr<Geometry::KDTree>		targetTree;
	std::vector<float>			targetPositionData;
	std::vector<unsigned int>	targetIndexData;
};

} // end namespace MeshOps

#endif
