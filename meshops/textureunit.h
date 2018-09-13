///-------------------------------------------------------------------------------------------------
/// \file	meshops\textureUnit.h
///
/// \brief	Declares the texture unit class.
///
/// \details	
///		textureUnit description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#if !defined( MESHOPS_TEXTUREUNIT_H_ )
#define MESHOPS_TEXTUREUNIT_H_

#include "layeredtexture.h"

namespace MeshOps {

//! How the raycaster should sample an attribute of the target mesh
enum SAMPLE_TYPE {
	/*! Sample the attribute directly of the surface, the attribute is 
		stored directly into the image map. */
	SAMPLE_DIRECT = 0,

	/*! Sample a texture using the attributes as UV coordinates, the texture image, and settings
	    are set via the AddTextureAttribute call. */
	SAMPLE_TEXTURE,

	/*! Sample a normal texture off the target mesh
		using the attributes as UV coordinates, the texture image, and settings
	    are set via the AddNormalTextureAttribute call. */
	SAMPLE_NORMAL_TEXTURE,
};

//! How the raycaster should filter a texture sample
enum TEXTURE_FILTER_TYPE {
	TEXTURE_FILTER_POINT = 0, 	//!< The sample is just point sample, picked off the texture and returned.
	TEXTURE_FILTER_BILINEAR,	//!< The sample is bilinearly filtered and returned.
};

//! How the raycaster should address a texture (there should be 1 per texture dimension)
enum TEXTURE_ADDRESS_TYPE {
	TEXTURE_ADDRESS_CLAMP = 0, //!< The texture address is clamp to 0-1
	TEXTURE_ADDRESS_WRAP,		//!< The texture is repeating around 0-1 (i.e. 1.1 = 0.1)
};

//! Class to encapsulate texture access
class TextureUnit {
public:
	TextureUnit(	float const*			pixels,
					unsigned int			componentCount,
					unsigned int			width,
					unsigned int			height,
					TEXTURE_FILTER_TYPE filterType,
					TEXTURE_ADDRESS_TYPE uAddressType,
					TEXTURE_ADDRESS_TYPE vAddressType );

	TextureUnit(	const LayeredFloatTexture& texture,
					const std::string& layerName, 
					TEXTURE_FILTER_TYPE filterType,
					TEXTURE_ADDRESS_TYPE uAddressType,
					TEXTURE_ADDRESS_TYPE vAddressType );

	void Sample( float u, float v, float outData[100] ) const;

private:
	void AddressModify( float& u, float& v ) const;
	void PointSample( float u, float v, float outData[100] ) const;
	void BilinearSample( float u, float v, float outData[100] ) const;

	TEXTURE_FILTER_TYPE	m_filterType; //!< how to filter from the texture
	TEXTURE_ADDRESS_TYPE	m_uAddressType; //!< how we address the texture sampler for u texture axis
	TEXTURE_ADDRESS_TYPE	m_vAddressType; //!< how we address the texture sampler for v texture axis
	float const*	m_pixels;				//!< float pointer to the dest image (cached for fast access)
	unsigned int 	m_componentCount;		//!< number of float per element
	unsigned int	m_width;				//!< width of the texture
	unsigned int	m_height;				//!< height of the texture

	float			m_uTexelOffset;			//!< half a texel in u direction
	float			m_vTexelOffset;			//!< half a texel in v direction

	// temp data, just used to optimise returning stuff internally
	mutable std::vector< float > m_tempData;		//!< this is use to hold a single pixels worth of sampled data
};

} // end namespace MeshOps

#endif
