///-------------------------------------------------------------------------------------------------
/// \file	meshops\textureunit.cpp
///
/// \brief	Implements the textureunit class.
///
/// \details	
///		textureunit description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshops.h"
#include "textureunit.h"
#include <algorithm>

namespace MeshOps {

TextureUnit::TextureUnit(	float const*	pixels,
							unsigned int	componentCount,
							unsigned int	width,
							unsigned int	height,
							TEXTURE_FILTER_TYPE filterType,
							TEXTURE_ADDRESS_TYPE uAddressType,
							TEXTURE_ADDRESS_TYPE vAddressType ) :
	m_pixels( pixels ),
	m_componentCount( componentCount ),
	m_width( width ),
	m_height( height ),
	m_filterType( filterType ),
	m_uAddressType( uAddressType ),
	m_vAddressType( vAddressType )
{
	m_tempData.resize( m_componentCount );
	m_uTexelOffset = 0.5f / float(m_width);
	m_vTexelOffset = 0.5f / float(m_height);
}

TextureUnit::TextureUnit( const LayeredTexture& texture,
	const std::string& layerName, 
	TEXTURE_FILTER_TYPE filterType,
	TEXTURE_ADDRESS_TYPE uAddressType,
	TEXTURE_ADDRESS_TYPE vAddressType ) :
	m_width( texture.getWidth() ),
	m_height( texture.getHeight() ),
	m_filterType( filterType ),
	m_uAddressType( uAddressType ),
	m_vAddressType( vAddressType ) {

	m_pixels = texture.getLayer(layerName).getData<float>();
	m_componentCount = texture.getLayer( layerName ).getComponentCount();

	m_tempData.resize( m_componentCount );
	m_uTexelOffset = 0.5f / float(m_width);
	m_vTexelOffset = 0.5f / float(m_height);
}

void TextureUnit::AddressModify( float& u, float& v ) const
{
	u += m_uTexelOffset;
	v += m_vTexelOffset;

	// Texture address system
	switch( m_uAddressType )
	{
	case TEXTURE_ADDRESS_CLAMP:
		u = std::min( 1.f, std::max( u, 0.f) );
		break;
	case TEXTURE_ADDRESS_WRAP:
		{
			float uint;
			u = std::modf( u, &uint );
		}
		break;
	}
	switch( m_vAddressType )
	{
	case TEXTURE_ADDRESS_CLAMP:
		v = std::min( 1.f, std::max( v, 0.f) );
		break;
	case TEXTURE_ADDRESS_WRAP:
		{
			float vint;
			v = std::modf( v, &vint );
		}
		break;
	}

	// safety
	u = std::min( 1.f, std::max( u, 0.f) );
	v = std::min( 1.f, std::max( v, 0.f) );

}

void TextureUnit::Sample( float u, float v, float outData[100] ) const
{
	switch( m_filterType )
	{
	case TEXTURE_FILTER_POINT:
		PointSample( u, v, outData );
		break;
	case TEXTURE_FILTER_BILINEAR:
		BilinearSample( u, v, outData );
		break;
	}
}

void TextureUnit::BilinearSample( float u, float v, float outData[100] ) const {
	float tempData[100];
	float dummy;
	float uf = std::modf( u, &dummy );
	float vf = std::modf( v, &dummy );

	float f0 = uf * vf;
	float f1 = (1.f - uf) * vf;
	float f2 = uf * (1.f - vf);
	float f3 = (1.f - uf) * (1.f - vf);

	PointSample( u, v, tempData );
	for( unsigned int i = 0; i < m_componentCount; ++i )
		outData[i] = tempData[i] * f3;

	PointSample( u+m_uTexelOffset, v, tempData );
	for( unsigned int i = 0; i < m_componentCount; ++i )
		outData[i] += tempData[i] * f2;

	PointSample( u, v+m_vTexelOffset, tempData );
	for( unsigned int i = 0; i < m_componentCount; ++i )
		outData[i] += tempData[i] * f1;

	PointSample( u+m_uTexelOffset, v+m_vTexelOffset, tempData );
	for( unsigned int i = 0; i < m_componentCount; ++i )
		outData[i] += tempData[i] * f0;
}

void TextureUnit::PointSample( float u, float v, float outData[100] ) const {
	float tu = u;
	float tv = v;
	AddressModify( tu, tv );

	// scale to pixel coordinates
	tu = tu * (m_width-1);
	tv = tv * (m_height-1); 

	// lookup the texture
	const unsigned int offset =	(unsigned int)( std::floor(tv) )* m_width +  (unsigned int)( std::floor(tu) );
	float const* image = m_pixels + offset * m_componentCount;
	for( unsigned int i = 0; i < m_componentCount; ++i ) {
		outData[i] = image[i];
	}
}


} // end namespace MeshOps