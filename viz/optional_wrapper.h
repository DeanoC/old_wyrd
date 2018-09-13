#pragma once
#ifndef CGEOMETRYENGINE_OPTIONAL_WRAPPER_H
#define CGEOMETRYENGINE_OPTIONAL_WRAPPER_H

#if __APPLE__
#include "TargetConditionals.h"
#include "Optional/optional.hpp"
#else
#include <optional>
namespace std
{
//	using experimental::optional;
}
#endif


#endif //CGEOMETRYENGINE_OPTIONAL_WRAPPER_H
