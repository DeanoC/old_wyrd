#pragma once
#ifndef WYRD_SIMPLEPADCAMERA_H
#define WYRD_SIMPLEPADCAMERA_H

#include "core/core.h"
#include "math/vector_math.h"
#include "input/vpadlistener.h"
#include "midrender/simpleeye.h"

struct SimplePadCamera : public Input::VPadListener
{
	SimplePadCamera()
	{
		simpleEye.setProjection(60.0f, 1280.0f / 720.0f, 0.01f);
		lookatPoint = Math::vec3(0, 0, 0);
		position = Math::vec3(0, 0, 4);
		update(0.0);
	}

	~SimplePadCamera() final
	{

	}

	void axisMovement(Input::VPadAxisId id_, float delta_) final
	{
		switch(id_)
		{
			case Input::VPadAxisId::LX:
				lookatPoint.x += delta_ * 0.1f;
				position.x += delta_ * 0.1f;
				break;
			case Input::VPadAxisId::LY:
				lookatPoint.z += delta_ * 0.1f;
				position.z += delta_ * 0.1f;
				break;

			default: break;

		}
	}

	void button(Input::VPadButtonId id_, float delta_) final
	{
	}

	auto update(double deltaT_) -> void
	{
		Math::mat4x4 view = Math::lookAt(position,
										 lookatPoint,
										 Math::vec3(0, 1, 0));
		simpleEye.setView(view);
	}

	Math::vec3 position;
	Math::vec3 lookatPoint;

	MidRender::SimpleEye simpleEye;
};

#endif //WYRD_SIMPLEPADCAMERA_H
