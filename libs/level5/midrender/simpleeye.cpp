#include "core/core.h"
#include "midrender/simpleeye.h"

namespace MidRender {

SimpleEye::SimpleEye() :
		ortho(false),
		zNear(0.1f)
{
	setProjection(90.0f, 1.0f, 0.1f);
	setView(Math::identity<Math::mat4x4>());
}

auto SimpleEye::setProjection(float fov_, float aspectWoverH_, float znear_) -> void
{
	fov = fov_;
	aspectWoverH = aspectWoverH_;
	zNear = znear_;
	ortho = false;
	setProjection();
}

auto SimpleEye::setOrthographic(float left_, float right_,
								float top_, float bottom_,
								float znear_) -> void
{
	left = left_;
	right = right_;
	top = top_;
	bottom = bottom_;
	zNear = znear_;
	ortho = true;
	setOrthographic();
}

auto SimpleEye::setFOV(float fov_, float aspectWoverH_) -> void
{
	fov = fov_;
	aspectWoverH = aspectWoverH_;
	ortho = false;
	setProjection();
}

auto SimpleEye::setZNear(float near_) -> void
{
	zNear = near_;

	if(ortho)
	{
		setOrthographic();
	} else
	{
		setProjection();
	}
}

auto SimpleEye::setProjection() -> void
{
//	projectionMatrix = Math::infinitePerspective(fov, aspect, zNear);
	float const fovY_radians = Math::degreesToRadians(fov);

	float const f = 1.0f / tan(fovY_radians / 2.0f);
	projectionMatrix = glm::mat4(
			f / aspectWoverH, 0.0f,  0.0f,  0.0f,
			0.0f,    f,  0.0f,  0.0f,
			0.0f, 0.0f,  0.0f, -1.0f,
			0.0f, 0.0f, zNear,  0.0f);
}

auto SimpleEye::setOrthographic() -> void
{
	projectionMatrix = Math::ortho(left, right, bottom, top, zNear, 1000000.0f);
}

auto SimpleEye::setView(Math::mat4x4 const& _viewMatrix) -> void
{
	viewMatrix = _viewMatrix;
}

auto SimpleEye::computeFrustum() const -> Geometry::Frustum
{
	return Geometry::Frustum(viewMatrix * projectionMatrix);
}

} // end namespace
