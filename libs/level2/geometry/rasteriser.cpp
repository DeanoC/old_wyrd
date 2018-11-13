#include "core/core.h"
#include "geometry/rasteriser.h"
#include <random>

namespace Geometry {

static inline float VanDerCorput(unsigned int n, unsigned int scramble)
{
	n = (n << 16) | (n >> 16);
	n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
	n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
	n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
	n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
	n ^= scramble;
	return float(n) / float(0x100000000uLL);
}

static inline float Sobol2(unsigned int n, unsigned int scramble)
{
	for(unsigned int v = 0x80000000; n != 0; n >>= 1, v ^= v >> 1)
	{
		if(n & 0x1)
			scramble ^= v;
	}
	return float(scramble) / float(0x100000000uLL);
}

Rasteriser::Rasteriser(unsigned int width, unsigned int height, unsigned int subSampleCount)
		: m_width(width),
		  m_height(height),
		  m_subSampleCount(subSampleCount),
		  m_left(0),
		  m_top(0),
		  m_right(0),
		  m_bottom(0),
		  m_baryConstRcp(1.0f),
		  m_x(0),
		  m_y(0),
		  m_sample(0)
{
	// check arguments
	assert(m_width >= 1 && m_height >= 1 && m_subSampleCount >= 1);

	// initialise our random number generators
	std::mt19937 generator;
	std::uniform_int_distribution<unsigned int> distribution(0, 0xffffffff);

	// generate two random numbers per pixel if we're multi-sampling
	if(m_subSampleCount > 1)
	{
		unsigned int const seedCount = 2 * m_width * m_height;
		m_sampleSeeds.resize(seedCount);
		for(unsigned int i = 0; i < seedCount; ++i)
			m_sampleSeeds[i] = distribution(generator);
	}
}

void Rasteriser::Reset(RasteriserTriangle const *triangle)
{
	// move the coordinates into image space
	for(unsigned int i = 0; i < 3; ++i)
	{
		m_current.x[i] = triangle->x[i] * float(m_width);
		m_current.y[i] = triangle->y[i] * float(m_height);
	}

	// set up for rasterisation
	float left = std::min(std::min(m_current.x[0], m_current.x[1]), m_current.x[2]);
	float right = std::max(std::max(m_current.x[0], m_current.x[1]), m_current.x[2]);
	float top = std::min(std::min(m_current.y[0], m_current.y[1]), m_current.y[2]);
	float bottom = std::max(std::max(m_current.y[0], m_current.y[1]), m_current.y[2]);

	// rely on casting doing truncation
	m_left = std::max((unsigned int) (0), (unsigned int) (left));
	m_right = std::min(m_width, (unsigned int) (right) + 1);
	m_top = std::max((unsigned int) (0), (unsigned int) (top));
	m_bottom = std::min(m_height, (unsigned int) (bottom) + 1);

	// pre-compute a barycentric coordinate constant
	m_baryConstRcp = 1.0f / (
			(m_current.x[1] - m_current.x[0]) * (m_current.y[2] - m_current.y[0])
			- (m_current.x[2] - m_current.x[0]) * (m_current.y[1] - m_current.y[0])
	);

	// reset the scanline
	m_x = m_left;
	m_y = m_top;
	m_sample = 0;
}

bool Rasteriser::GetNext(RasteriserPixel *pixel) const
{
	// continue the loop
	while(m_y < m_bottom)
	{
		// continue the scanline
		while(m_x < m_right)
		{
			// get the pixel location
			unsigned int const offset = m_y * m_width + m_x;

			// continue sampling this pixel
			while(m_sample < m_subSampleCount)
			{
				// get the exact sample coordinate
				float x = float(m_x);
				float y = float(m_y);
				if(!m_sampleSeeds.empty())
				{
					x += VanDerCorput(m_sample, m_sampleSeeds[2 * offset]);
					y += Sobol2(m_sample, m_sampleSeeds[2 * offset + 1]);
				} else
				{
					x += float(0.5);
					y += float(0.5);
				}

				// advance
				++m_sample;

				// test this point to see if it's in the triangle
				float v = m_baryConstRcp * (
						(m_current.x[2] - x) * (m_current.y[0] - y)
						- (m_current.x[0] - x) * (m_current.y[2] - y)
				);

				// early out if we can
				if(0.0f <= v && v <= 1.0f)
				{
					float w = m_baryConstRcp * (
							(m_current.x[0] - x) * (m_current.y[1] - y)
							- (m_current.x[1] - x) * (m_current.y[0] - y)
					);
					/*float u = m_baryConstRcp*( 
						( m_current.x[1] - x )*( m_current.y[2] - y )
						- ( m_current.x[2] - x )*( m_current.y[1] - y )
					);*/

					// check for a hit
					if(0.0f <= w && w + v <= 1.0f)
					{
						// store
						pixel->x = m_x;
						pixel->y = m_y;
						pixel->v = v;
						pixel->w = w;

						// done
						return true;
					}
				}
			}

			// advance a pixel
			++m_x;
			m_sample = 0;
		}

		// advance a scanline
		++m_y;
		m_x = 0;
	}

	// no hit
	return false;
}

} // namespace internal
