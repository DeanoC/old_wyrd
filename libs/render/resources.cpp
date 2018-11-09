#include "core/core.h"
#include "binny/writehelper.h"
#include "fmt/format.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/writer.h"
#include "render/resources.h"
#include "render/image.h"
#include "render/texture.h"
#include "render/renderpass.h"
#include "render/rendertarget.h"
#include "render/shader.h"
#include "render/pipeline.h"
#include "render/ropblender.h"
#include "render/viewport.h"
#include "render/vertexinput.h"

namespace Render {

auto RegisterResourceHandlers(ResourceManager::ResourceMan& rm_) -> void
{
	using namespace ResourceManager;

	GenericImage::RegisterResourceHandler(rm_);
	Texture::RegisterResourceHandler(rm_);
	RenderPass::RegisterResourceHandler(rm_);
	RenderTarget::RegisterResourceHandler(rm_);
	SPIRVShader::RegisterResourceHandler(rm_);
	RenderPipeline::RegisterResourceHandler(rm_);
	ComputePipeline::RegisterResourceHandler(rm_);
	ROPBlender::RegisterResourceHandler(rm_);
	Viewport::RegisterResourceHandler(rm_);
	VertexInput::RegisterResourceHandler(rm_);
}

} // end namespace