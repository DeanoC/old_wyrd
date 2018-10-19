#pragma once
#ifndef WYRD_FILESYSTEM_H
#define WYRD_FILESYSTEM_H

#include "core/core.h"
#if PLATFORM_OS == OSX
#include "macos/experimental/filesystem"
#else
#include <experimental/filesystem>
#endif

#endif //WYRD_FILESYSTEM_H