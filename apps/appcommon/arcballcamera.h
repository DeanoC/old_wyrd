#pragma once

#ifndef WYRD_ARCBALLCAMERA_H
#define WYRD_ARCBALLCAMERA_H

#include "core/core.h"
#include "math/vector_math.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "midrender/simpleeye.h"

struct ArcBallCamera
{
	ArcBallCamera(Math::vec3 default_ = Math::vec3(0,0,0), double distance_ = 400.0)
	{
		simpleEye.setProjection(60.0f, 1280.0f / 720.0f, 0.01f);
		lookatPoint = default_;
		distance = distance_;
		update(0.0);
	}

	void update(double deltaT_)
	{
		using namespace Input;
		using namespace Math;

		bool altDown = false;
		if (g_Keyboard != nullptr)
		{
			altDown = g_Keyboard->keyDown(Key::KT_LALT);
		}

		if (g_Mouse != nullptr)
		{
			float mouseVertical = g_Mouse->getMouseWheelVertical();
			if (altDown)
			{
				distance += 100.0 * deltaT_;
			}

			if (altDown && g_Mouse->buttonDown(MouseButton::Left))
			{
				// first click not held, record the position
				if (!g_Mouse->buttonHeld(MouseButton::Left))
				{
					lmbIsDown = true;
					lmbDownStartPosX = g_Mouse->getAbsMouseX();
					lmbDownStartPosY = g_Mouse->getAbsMouseY();
				}
			}

			if (lmbIsDown)
			{
				double dmx = g_Mouse->getAbsMouseX() - lmbDownStartPosX;
				double dmy = g_Mouse->getAbsMouseY() - lmbDownStartPosY;

				lmbDownStartPosX = g_Mouse->getAbsMouseX();
				lmbDownStartPosY = g_Mouse->getAbsMouseY();

				dmx = dmx * float(10 * deltaT_);
				dmy = dmy * float(10 * deltaT_);

				yaw = yaw + dmx;
				pitch = pitch + dmy;

				if (g_Mouse->buttonUp(MouseButton::Left))
				{
					// end of the drag operation
					lmbIsDown = false;
				}
			}
		}

		mat4x4 rotation = Math::yawPitchRoll((float)yaw, (float)pitch, (float)roll);

		vec3 right = Math::normalize(rotation[0]);
		vec3 up = Math::normalize(rotation[1]);
		vec3 forward = Math::normalize(rotation[2]);

		if (g_Keyboard != nullptr)
		{

			if (g_Keyboard->keyDown(Key::KT_W))
			{
				lookatPoint += forward * float(100.0 * deltaT_);
			}
			if (g_Keyboard->keyDown(Key::KT_S))
			{
				lookatPoint += forward * float(100.0 * deltaT_);
			}
			if(g_Keyboard->keyDown(Key::KT_A))
			{
				lookatPoint += right * float(100.0 * deltaT_);
			}
			if(g_Keyboard->keyDown(Key::KT_D))
			{
				lookatPoint -= right * float(100.0 * deltaT_);
			}
		}

		vec3 pos = lookatPoint - (forward * (float)distance);
		mat4x4 translation = translate(identity<mat4x4>(), pos);
		mat4x4 view = transpose(rotation) * translation;
		simpleEye.setView(view);
	}

	Math::vec3 lookatPoint;

	double distance;
	double yaw = 0.0f;
	double pitch = 0.0f;
	double roll = 0.0f;

	bool lmbIsDown = false;
	float lmbDownStartPosX;
	float lmbDownStartPosY;

	MidRender::SimpleEye simpleEye;
};
#endif //WYRD_ARCBALLCAMERA_H
