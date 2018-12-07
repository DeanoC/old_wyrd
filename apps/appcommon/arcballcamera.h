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
	ArcBallCamera()
	{
		simpleEye.setProjection(60.0f, 1280.0f / 720.0f, 0.01f);
		lookatPoint = Math::vec3(0, 0, 0);
		position = Math::vec3(0, 0, 4);
		update(0.0);
	}

	void update(double deltaT_)
	{
		using namespace Input;
		using namespace Math;

		mat4x4 lookAtMatrix = lookAt(position,
									lookatPoint,
									vec3(0, 1, 0));

		vec3 right = lookAtMatrix[0];
		vec3 up = lookAtMatrix[1];
		vec3 forward = lookAtMatrix[2];

		bool altDown = false;
		if(g_Keyboard != nullptr)
		{
			altDown = g_Keyboard->keyDown(Key::KT_LALT);

			if(g_Keyboard->keyDown(Key::KT_W))
			{
				auto t = float(10.0 * deltaT_);
				position += forward * t;
				lookatPoint += forward * t;
			}
			if(g_Keyboard->keyDown(Key::KT_S))
			{
				auto t = float(10.0 * deltaT_);
				position -= forward * t;
				lookatPoint -= forward * t;
			}

			if(g_Keyboard->keyDown(Key::KT_A))
			{
				auto t = float(100.0 * deltaT_);
				position += right * t;
				lookatPoint += right * t;
			}
			if(g_Keyboard->keyDown(Key::KT_D))
			{
				auto t = float(100.0 * deltaT_);
				position -= right * t;
				lookatPoint -= right * t;
			}
		}
		vec3 pos = position;
		vec3 lap = lookatPoint;

		if(g_Mouse != nullptr)
		{
			float mouseVertical = g_Mouse->getMouseWheelVertical();

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
				float dmx = g_Mouse->getAbsMouseX() - lmbDownStartPosX;
				float dmy = g_Mouse->getAbsMouseY() - lmbDownStartPosY;

				dmx = dmx * float(10.0 * deltaT_);
				dmy = dmy * float(10.0 * deltaT_);

				mat4x4 center = translate(
					Math::identity<Math::mat4x4>(),
					-lookatPoint
				);
				mat4x4 rotX = rotate(
					Math::identity<Math::mat4x4>(),
					dmx,
					vec3(0, 1, 0));

				mat4x4 rotY = rotate(
					Math::identity<Math::mat4x4>(),
					dmy,
					vec3(1, 0, 0));

				lookAtMatrix = (rotX * rotY) * lookAtMatrix;
				pos = -lookAtMatrix[3];

				if(g_Mouse->buttonUp(MouseButton::Left))
				{
					// end of the drag operation
					lmbIsDown = false;
					lmbDownStartPosX = 0;
					lmbDownStartPosY = 0;
					position = pos;
					lookatPoint = lap;
				}
			}
		}

		simpleEye.setView(Math::lookAt(pos,
										lap,
										vec3(0, 1, 0)));
	}

	Math::vec3 position;
	Math::vec3 lookatPoint;

	bool lmbIsDown = false;
	float lmbDownStartPosX;
	float lmbDownStartPosY;

	MidRender::SimpleEye simpleEye;
};
#endif //WYRD_ARCBALLCAMERA_H
