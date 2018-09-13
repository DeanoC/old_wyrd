///-------------------------------------------------------------------------------------------------
/// \file	core\rasteriser.h
///
/// \brief	Declares the rasteriser class. 
///
/// \details	
///		rasteriser description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. meshtools license (below)
/// \remark	mailto://deano@cloudpixies.com
///
// Some portions are borrowed from meshtools and those portions have this copyright
/*
	Copyright (c) 2004 Simon Brown

	Permission is hereby granted, free of charge, to any person obtaining 
	a copy of this software and associated documentation files (the 
	"Software"), to deal in the Software without restriction, including 
	without limitation the rights to use, copy, modify, merge, publish, 
	distribute, sublicense, and/or sell copies of the Software, and to 
	permit persons to whom the Software is furnished to do so, subject to 
	the following conditions:

	The above copyright notice and this permission notice shall be 
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

	------------------------------------------------------------------------- */
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORE_RASTERISER_H
#define CORE_RASTERISER_H

#include <vector>

namespace Core {

//! The structure of an input triangle to the rasteriser.
struct RasteriserTriangle {
	float x[3];	//!< The triangle x co-ordinates.
	float y[3];	//!< The triangle y co-ordinates.
};

//! The structure of an output pixel from the rasteriser.
struct RasteriserPixel
{
	unsigned int x, y;	//!< The co-ordinate of the target pixel.
	float v, w;			//!< The v and w barycentric co-ordinate on the triangle.
};

///-------------------------------------------------------------------------------------------------
/// \class	Rasterisor
///
/// \brief	Rasterisor is a generic triangle rasterisor. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
class Rasteriser {
public:
	//! Creates a new rasteriser for the given framebuffer size.
	Rasteriser( unsigned int width, unsigned int height, unsigned int subSampleCount );

	//! Resets the rasteriser with a new triangle.
	void Reset( RasteriserTriangle const* triangle );

	//! Gets the next pixel from the current triangle.
	bool GetNext( RasteriserPixel* pixel ) const;

private:
	unsigned int m_width, m_height, m_subSampleCount;
	std::vector< unsigned int > m_sampleSeeds;

	RasteriserTriangle m_current;
	unsigned int m_left, m_top, m_right, m_bottom;
	float m_baryConstRcp;

	mutable unsigned int m_x, m_y, m_sample;
};

} // end of namespace Core

#endif