#pragma once
#ifndef WYRD_MIDRENDER_SIMPLEFORWARDGLOBALS_H
#define WYRD_MIDRENDER_SIMPLEFORWARDGLOBALS_H

namespace MidRender
{
// this structure contains various things most forward render shader will want
// should be filled once per frame and put into the binding table of whatever
// shaders need it
struct SimpleForwardGlobals
{
	float viewMatrix[16];
	float projectionMatrix[16];
	float viewProjectionMatrix[16];
};

}

#endif //WYRD_MIDRENDER_SIMPLEFORWARDGLOBALS_H
