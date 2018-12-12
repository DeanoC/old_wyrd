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
		position = Math::vec3(0, 0, 0);
		update(0.0);
	}

	~SimplePadCamera() final
	{

	}

	void axisMovement(Input::VPadAxisId id_, float delta_) final
	{
		if (!enabled) return;
		switch(id_)
		{
			case Input::VPadAxisId::LX:
				dx += -slideSpeed * delta_;
				break;
			case Input::VPadAxisId::LY:
				dy += forwardSpeed * delta_;
				break;
			case Input::VPadAxisId::RX:
				rx += -rotationSpeed * delta_;
				break;
			case Input::VPadAxisId::RY:
				ry += -rotationSpeed * delta_;
				break;

			default: break;

		}
	}

	void button(Input::VPadButtonId id_, float delta_) final
	{
		if (!enabled) return;
	}

	auto update(double deltaT_) -> void
	{
		if (!enabled) return;

		using namespace Input;
		using namespace Math;

		yaw += (rx * deltaT_);
		pitch += (ry * deltaT_);

		mat4x4 rotation = Math::yawPitchRoll((float)yaw, (float)pitch, (float)roll);

		vec3 right = Math::normalize(rotation[0]);
		vec3 up = Math::normalize(rotation[1]);
		vec3 forward = Math::normalize(rotation[2]);
		position += forward * float(dy * deltaT_);
		position += right * float(dx * deltaT_);

		simpleEye.setView(lookAt(position, position + forward, up));

		dx = dy = 0.0;
		rx = ry = 0.0;
	}

	bool enabled = true;
	Math::vec3 position;
	double dx = 0.0;
	double dy = 0.0;
	double rx = 0.0;
	double ry = 0.0;

	double yaw = 0.0;
	double pitch = 0.0;
	double roll = 0.0;

	double forwardSpeed = 25.0;
	double slideSpeed = 10.0;
	double rotationSpeed = 0.5;

	MidRender::SimpleEye simpleEye;
};

#endif //WYRD_SIMPLEPADCAMERA_H
