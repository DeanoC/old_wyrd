//!-----------------------------------------------------
//!
//! \file vector_maths.cpp
//! some maths functions
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "vector_math.h"

namespace {
float DetInternal(float a1, float a2, float a3,
				  float b1, float b2, float b3,
				  float c1, float c2, float c3)
{
	float ret = a1 * (b2 * c3 - b3 * c2) -
				b1 * (a2 * c3 - a3 * c2) +
				c1 * (a2 * b3 - a3 * b2);

	return ret;
}

}

namespace Math {
float C_Determinant(const Matrix4x4& mat)
{
	float a1, a2, a3, a4;
	float b1, b2, b3, b4;
	float c1, c2, c3, c4;
	float d1, d2, d3, d4;

	a1 = mat(0, 0);
	b1 = mat(0, 1);
	c1 = mat(0, 2);
	d1 = mat(0, 3);
	a2 = mat(1, 0);
	b2 = mat(1, 1);
	c2 = mat(1, 2);
	d2 = mat(1, 3);
	a3 = mat(2, 0);
	b3 = mat(2, 1);
	c3 = mat(2, 2);
	d3 = mat(2, 3);
	a4 = mat(3, 0);
	b4 = mat(3, 1);
	c4 = mat(3, 2);
	d4 = mat(3, 3);

	float ret = a1 * DetInternal(b2, b3, b4, c2, c3, c4, d2, d3, d4) -
				b1 * DetInternal(a2, a3, a4, c2, c3, c4, d2, d3, d4) +
				c1 * DetInternal(a2, a3, a4, b2, b3, b4, d2, d3, d4) -
				d1 * DetInternal(a2, a3, a4, b2, b3, b4, c2, c3, c4);

	return ret;
}

Matrix4x4 C_Adjoint(const Matrix4x4& mat)
{
	float a1, a2, a3, a4;
	float b1, b2, b3, b4;
	float c1, c2, c3, c4;
	float d1, d2, d3, d4;

	a1 = mat(0, 0);
	b1 = mat(0, 1);
	c1 = mat(0, 2);
	d1 = mat(0, 3);
	a2 = mat(1, 0);
	b2 = mat(1, 1);
	c2 = mat(1, 2);
	d2 = mat(1, 3);
	a3 = mat(2, 0);
	b3 = mat(2, 1);
	c3 = mat(2, 2);
	d3 = mat(2, 3);
	a4 = mat(3, 0);
	b4 = mat(3, 1);
	c4 = mat(3, 2);
	d4 = mat(3, 3);

	Matrix4x4 ret;
	ret(0, 0) = DetInternal(b2, b3, b4, c2, c3, c4, d2, d3, d4);
	ret(1, 0) = -DetInternal(a2, a3, a4, c2, c3, c4, d2, d3, d4);
	ret(2, 0) = DetInternal(a2, a3, a4, b2, b3, b4, d2, d3, d4);
	ret(3, 0) = -DetInternal(a2, a3, a4, b2, b3, b4, c2, c3, c4);

	ret(0, 1) = -DetInternal(b1, b3, b4, c1, c3, c4, d1, d3, d4);
	ret(1, 1) = DetInternal(a1, a3, a4, c1, c3, c4, d1, d3, d4);
	ret(2, 1) = -DetInternal(a1, a3, a4, b1, b3, b4, d1, d3, d4);
	ret(3, 1) = DetInternal(a1, a3, a4, b1, b3, b4, c1, c3, c4);

	ret(0, 2) = DetInternal(b1, b2, b4, c1, c2, c4, d1, d2, d4);
	ret(1, 2) = -DetInternal(a1, a2, a4, c1, c2, c4, d1, d2, d4);
	ret(2, 2) = DetInternal(a1, a2, a4, b1, b2, b4, d1, d2, d4);
	ret(3, 2) = -DetInternal(a1, a2, a4, b1, b2, b4, c1, c2, c4);

	ret(0, 3) = -DetInternal(b1, b2, b3, c1, c2, c3, d1, d2, d3);
	ret(1, 3) = DetInternal(a1, a2, a3, c1, c2, c3, d1, d2, d3);
	ret(2, 3) = -DetInternal(a1, a2, a3, b1, b2, b3, d1, d2, d3);
	ret(3, 3) = DetInternal(a1, a2, a3, b1, b2, b3, c1, c2, c3);

	return ret;

}

Matrix4x4 C_Invert(const Matrix4x4& mat)
{
	float det = C_Determinant(mat);
	Matrix4x4 ret;

	if(fabsf(det) < 1e-11f)
		ret = IdentityMatrix();
	else
	{
		ret = C_Adjoint(mat);
		ret = ret * (1.f / det);
	}

	return ret;
}

static uint32_t const SRGBTable[256] = {
		0x00000000, 0x399f22b4, 0x3a1f22b4, 0x3a6eb40e, 0x3a9f22b4, 0x3ac6eb61, 0x3aeeb40e, 0x3b0b3e5d,
		0x3b1f22b4, 0x3b33070b, 0x3b46eb61, 0x3b5b518d, 0x3b70f18d, 0x3b83e1c6, 0x3b8fe616, 0x3b9c87fd,
		0x3ba9c9b7, 0x3bb7ad6f, 0x3bc63549, 0x3bd56361, 0x3be539c1, 0x3bf5ba70, 0x3c0373b5, 0x3c0c6152,
		0x3c15a703, 0x3c1f45be, 0x3c293e6b, 0x3c3391f7, 0x3c3e4149, 0x3c494d43, 0x3c54b6c7, 0x3c607eb1,
		0x3c6ca5df, 0x3c792d22, 0x3c830aa8, 0x3c89af9f, 0x3c9085db, 0x3c978dc5, 0x3c9ec7c2, 0x3ca63433,
		0x3cadd37d, 0x3cb5a601, 0x3cbdac20, 0x3cc5e639, 0x3cce54ab, 0x3cd6f7d5, 0x3cdfd010, 0x3ce8ddb9,
		0x3cf2212c, 0x3cfb9ac1, 0x3d02a569, 0x3d0798dc, 0x3d0ca7e6, 0x3d11d2af, 0x3d171963, 0x3d1c7c2e,
		0x3d21fb3c, 0x3d2796b2, 0x3d2d4ebb, 0x3d332380, 0x3d39152b, 0x3d3f23e3, 0x3d454fd1, 0x3d4b991c,
		0x3d51ffef, 0x3d58846a, 0x3d5f26b7, 0x3d65e6fe, 0x3d6cc564, 0x3d73c20f, 0x3d7add29, 0x3d810b67,
		0x3d84b795, 0x3d887330, 0x3d8c3e4a, 0x3d9018f6, 0x3d940345, 0x3d97fd4a, 0x3d9c0716, 0x3da020bb,
		0x3da44a4b, 0x3da883d7, 0x3daccd70, 0x3db12728, 0x3db59112, 0x3dba0b3b, 0x3dbe95b5, 0x3dc33092,
		0x3dc7dbe2, 0x3dcc97b6, 0x3dd1641f, 0x3dd6412c, 0x3ddb2eef, 0x3de02d77, 0x3de53cd5, 0x3dea5d19,
		0x3def8e52, 0x3df4d091, 0x3dfa23e8, 0x3dff8861, 0x3e027f07, 0x3e054280, 0x3e080ea3, 0x3e0ae378,
		0x3e0dc105, 0x3e10a754, 0x3e13966b, 0x3e168e52, 0x3e198f10, 0x3e1c98ad, 0x3e1fab30, 0x3e22c6a3,
		0x3e25eb09, 0x3e29186c, 0x3e2c4ed0, 0x3e2f8e41, 0x3e32d6c4, 0x3e362861, 0x3e39831e, 0x3e3ce703,
		0x3e405416, 0x3e43ca5f, 0x3e4749e4, 0x3e4ad2ae, 0x3e4e64c2, 0x3e520027, 0x3e55a4e6, 0x3e595303,
		0x3e5d0a8b, 0x3e60cb7c, 0x3e6495e0, 0x3e6869bf, 0x3e6c4720, 0x3e702e0c, 0x3e741e84, 0x3e781890,
		0x3e7c1c38, 0x3e8014c2, 0x3e82203c, 0x3e84308d, 0x3e8645ba, 0x3e885fc5, 0x3e8a7eb2, 0x3e8ca283,
		0x3e8ecb3d, 0x3e90f8e1, 0x3e932b74, 0x3e9562f8, 0x3e979f71, 0x3e99e0e2, 0x3e9c274e, 0x3e9e72b7,
		0x3ea0c322, 0x3ea31892, 0x3ea57308, 0x3ea7d289, 0x3eaa3718, 0x3eaca0b7, 0x3eaf0f69, 0x3eb18333,
		0x3eb3fc18, 0x3eb67a18, 0x3eb8fd37, 0x3ebb8579, 0x3ebe12e1, 0x3ec0a571, 0x3ec33d2d, 0x3ec5da17,
		0x3ec87c33, 0x3ecb2383, 0x3ecdd00b, 0x3ed081cd, 0x3ed338cc, 0x3ed5f50b, 0x3ed8b68d, 0x3edb7d54,
		0x3ede4965, 0x3ee11ac1, 0x3ee3f16b, 0x3ee6cd67, 0x3ee9aeb6, 0x3eec955d, 0x3eef815d, 0x3ef272ba,
		0x3ef56976, 0x3ef86594, 0x3efb6717, 0x3efe6e02, 0x3f00bd2d, 0x3f02460e, 0x3f03d1a7, 0x3f055ff9,
		0x3f06f106, 0x3f0884cf, 0x3f0a1b56, 0x3f0bb49b, 0x3f0d50a0, 0x3f0eef67, 0x3f1090f1, 0x3f12353e,
		0x3f13dc51, 0x3f15862b, 0x3f1732cd, 0x3f18e239, 0x3f1a946f, 0x3f1c4971, 0x3f1e0141, 0x3f1fbbdf,
		0x3f21794e, 0x3f23398e, 0x3f24fca0, 0x3f26c286, 0x3f288b41, 0x3f2a56d3, 0x3f2c253d, 0x3f2df680,
		0x3f2fca9e, 0x3f31a197, 0x3f337b6c, 0x3f355820, 0x3f3737b3, 0x3f391a26, 0x3f3aff7c, 0x3f3ce7b5,
		0x3f3ed2d2, 0x3f40c0d4, 0x3f42b1be, 0x3f44a590, 0x3f469c4b, 0x3f4895f1, 0x3f4a9282, 0x3f4c9201,
		0x3f4e946e, 0x3f5099cb, 0x3f52a218, 0x3f54ad57, 0x3f56bb8a, 0x3f58ccb0, 0x3f5ae0cd, 0x3f5cf7e0,
		0x3f5f11ec, 0x3f612eee, 0x3f634eef, 0x3f6571e9, 0x3f6797e3, 0x3f69c0d6, 0x3f6beccd, 0x3f6e1bbf,
		0x3f704db8, 0x3f7282af, 0x3f74baae, 0x3f76f5ae, 0x3f7933b9, 0x3f7b74c6, 0x3f7db8e0, 0x3f800000
};

float SRGB_to_float(uint32_t val)
{
	return *(float *) &SRGBTable[val];
}

float SRGB_to_float(float val)
{
	return *(float *) &SRGBTable[(uint8_t) (Clamp(val, 0.0f, 1.0f) * 255.1f)];
}

Plane CreatePlaneFromPoints(size_t pointCount, Vector3 const* points)
{
	assert(pointCount >= 3);

	// calculate centroid of point cloud
	Vector3 centroid = points[0];
	for (size_t i = 1; i < pointCount; i++)
	{
		centroid += points[i];
	}
	centroid = centroid / (float)pointCount;

	// calculate covariance matrix (a symetrical matrix)
	double covMat[6] = { 0, 0, 0, 0, 0, 0 };
	for (size_t i = 0; i < pointCount; i++)
	{
		Vector3 rel = points[i] - centroid;
		covMat[0] += rel.x * rel.x;
		covMat[1] += rel.x * rel.y;
		covMat[2] += rel.x * rel.z;
		covMat[3] += rel.y * rel.y;
		covMat[4] += rel.y * rel.z;
		covMat[5] += rel.z * rel.z;
	}
	// re-normalise covariance matrix
	for (double& v : covMat) v /= (double)pointCount;

	double const xx = covMat[0];
	double const xy = covMat[1];
	double const xz = covMat[2];
	double const yy = covMat[3];
	double const yz = covMat[4];
	double const zz = covMat[5];

	// solve linear regression along the cardinal axises and 
	// weight them based on the matrix determinant
	// comes close to the 'proper' solving the eigen system

	double wd[3] = { 0, 0, 0 };
	{
		double const detx = yy * zz - square(yz);
		double const xaxis[3] = { detx, xz * yz - yz * zz, xy * yz - xz * yy };
		double const dx = wd[0] * xaxis[0] + wd[1] * xaxis[1] + wd[2] * xaxis[2];
		double const wx = (dx < 0.0) ? -square(detx) : square(detx);
		wd[0] += xaxis[0] * wx;
		wd[1] += xaxis[1] * wx;
		wd[2] += xaxis[2] * wx;
	}
	{
		double const dety = xx * zz - square(xz);
		double const yaxis[3] = { xz * yz - xy * zz, dety, xy * xz - yz * xx };
		double const dy = wd[0] * yaxis[0] + wd[1] * yaxis[1] + wd[2] * yaxis[2];
		double const wy = (dy < 0.0) ? -square(dety) : square(dety);
		wd[0] += yaxis[0] * wy;
		wd[1] += yaxis[1] * wy;
		wd[2] += yaxis[2] * wy;
	}
	{
		double const detz = xx * yy - square(xy);
		double const zaxis[3] = { xy * yz - xz * yy, xy * xz - yz * zz, detz };
		double const dz = wd[0] * zaxis[0] + wd[1] * zaxis[1] + wd[2] * zaxis[2];
		double const wz = (dz < 0.0) ? -square(detz) : square(detz);
		wd[0] += zaxis[0] * wz;
		wd[1] += zaxis[1] * wz;
		wd[2] += zaxis[2] * wz;
	}

	Vector3 weightedDir;

	// no dominate axis, try normalized centroid
	double lensqr = wd[0] * wd[0] + wd[1] * wd[1] + wd[2] * wd[2];
	if(lensqr < 1e-15f )
	{
		if (ApproxEqual(centroid, Vector3(0, 0, 0)))
		{
			// nothing to do but invent one...
			weightedDir = Vector3(0, 1, 0);
		}
		else
		{
			weightedDir = Normalise(centroid);
		}
	}
	else
	{
		double len = sqrt(lensqr);
		weightedDir.x = (float)(wd[0] / len);
		weightedDir.y = (float)(wd[1] / len);
		weightedDir.z = (float)(wd[2] / len);

	}
	return Plane(weightedDir, Dot(weightedDir, centroid));
 }

};