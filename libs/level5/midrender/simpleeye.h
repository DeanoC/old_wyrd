#pragma  once
#ifndef WYRD_MIDRENDER_SIMPLEEYE_H
#define WYRD_MIDRENDER_SIMPLEEYE_H

#include "core/core.h"
#include "math/vector_math.h"
#include "geometry/frustum.h"

namespace MidRender {

class SimpleEye
{
public:
	SimpleEye();

	//! sets the projection of the camera, fov is total not half, reverse z float infinite far project
	auto setProjection(float fov_, float aspectWoverH_, float znear_) -> void;
	auto setOrthographic(float left_, float right_, float top_, float bottom_, float znear_) -> void;

	auto setFOV(float fov_, float aspectWoverH_ = 1.0f) -> void;
	auto setZNear(float near_) -> void;

	auto setView(const Math::mat4x4& viewMatrix) -> void;

	auto computeFrustum() const -> Geometry::Frustum;

	auto getProjection() const -> Math::mat4x4 const& { return projectionMatrix; }

	auto getView() const -> Math::mat4x4 const& { return viewMatrix; }

	auto getZNear() const { return zNear; };

protected:
	auto setProjection() -> void; // uses already stored values
	auto setOrthographic() -> void; // uses already stored values

	Math::mat4x4 viewMatrix;
	Math::mat4x4 projectionMatrix;
	bool ortho;
	float left;
	float right;
	float top;
	float bottom;

	float fov;
	float aspectWoverH;
	float zNear;
};

}

#endif //WYRD_MIDRENDER_SIMPLEEYE_H
