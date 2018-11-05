#include "core/core.h"
#include "binny/writehelper.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/writer.h"
#include "render/resources.h"
#include "render/image.h"
#include "render/texture.h"
#include "render/renderpass.h"
#include "render/rendertarget.h"
#include "fmt/format.h"

namespace Render {

auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_) -> void
{
	using namespace ResourceManager;

	GenericImage::RegisterResourceHandler(rm_);
	Texture::RegisterResourceHandler(rm_);
	RenderPass::RegisterResourceHandler(rm_);
	RenderTarget::RegisterResourceHandler(rm_);

}

} // end namespace