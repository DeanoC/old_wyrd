#include "core/core.h"
#include "api.h"

namespace Vulkan
{
#define GLOBAL_VK_FUNC( name ) PFN_##name name;
#define INSTANCE_VK_FUNC( name ) PFN_##name name;
#define INSTANCE_VK_FUNC_EXT( name, extension ) PFN_##name name;
#include "functionlist.inl"
} // namespace Vulkan