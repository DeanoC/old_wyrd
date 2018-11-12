
#include <render/shader.h>
#include "core/core.h"
#include "midrender/stocks.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "render/shader.h"
#include "render/texture.h"
#include "render/types.h"
#include "render/rasterisationstate.h"
#include "render/renderpass.h"
#include "render/rendertarget.h"
#include "render/ropblender.h"
#include "render/vertexinput.h"
#include "render/viewport.h"

namespace MidRender {

auto Stocks::InitBasics(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void
{
	using namespace Render;
	using namespace ResourceManager;
	using namespace std::literals;
	using namespace Core::bitmask;


	auto const glslPassthroughSourceName = ResourceNameView("mem$glslPassthroughShaderSource"sv);
	TextResource::Create(rm_,
						 glslPassthroughSourceName,
						 "#extension GL_ARB_separate_shader_objects : enable\n"
						 "#extension GL_ARB_shading_language_420pack : enable\n"
						 "layout (location = 0) in vec4 pos;\n"
						 "void main() {\n"
						 "   gl_Position = pos;\n"
						 "}\n");

	SPIRVShader::Compile(rm_,
						 Stock::passthroughVertexShader,
						 {rm_->openByName<TextResourceId>(glslPassthroughSourceName)},
						 ShaderSourceLanguage::GLSL,
						 ShaderType::Vertex,
						 0);

	RenderPass::Create(rm_,
					   Stock::defaultRenderPass,
					   {{
								LoadOp::Clear,
								StoreOp::Store,
								GenericTextureFormat::R8G8B8A8_UNORM,
						}},
					   {{0.0f, 0.0f, 0.0f, 1.0f}});

	VertexInput::Create(rm_,
						Stock::positionOnlyVertexInput,
						{
								{0, 0, VertexInputElement::Position, VertexInputType::Float3}
						});

	ROPBlender::Create(rm_,
					   Stock::defaultROPBlender,
					   {
							   {
									   ROPTargetBlender::EnableFlag,
									   ROPBlendOps::Add,
									   ROPBlendOps::Add,
									   ColourComponents::All,
									   ROPBlendFactor::SrcColour,
									   ROPBlendFactor::Zero,
									   ROPBlendFactor::SrcAlpha,
									   ROPBlendFactor::Zero
							   }
					   },
					   {0.0f, 0.0f, 0.0f, 0.0f}
	);

	RasterisationState::Create(rm_, Stock::defaultRasterState);

	Texture::Create(rm_,
					Stock::blank4x4Tex,
					TextureFlags::InitZero |
					Texture::FromUsage(Usage::ShaderRead | Usage::DMADst),
					4, 4, 1, 1,
					1, 1,
					GenericTextureFormat::R8G8B8A8_UNORM);

}


auto Stocks::InitSimpleForwardRenderer(std::shared_ptr<ResourceManager::ResourceMan> const& rm_, uint32_t width_,
									   uint32_t height_) -> void
{
	using namespace Render;
	using namespace ResourceManager;
	using namespace std::literals;
	using namespace Core::bitmask;

	Texture::Create(rm_,
					Stock::simpleForwardRendererColourRT,
					TextureFlags::NoInit |
					Texture::FromUsage(Usage::RopRead | Usage::RopWrite | Usage::DMASrc | Usage::DMADst),
					width_, height_, 1, 1,
					1, 1,
					GenericTextureFormat::R8G8B8A8_UNORM);

	Texture::Create(rm_,
					Stock::simpleForwardRendererDepthStencilRT,
					TextureFlags::NoInit |
					Texture::FromUsage(Usage::RopRead | Usage::RopWrite),
					width_, height_, 1, 1,
					1, 1,
					GenericTextureFormat::D32_SFLOAT_S8_UINT);

	Viewport::Create(rm_,
					 Stock::simpleForwardRendererViewport,
					 {{
							  0, 0,
							  (float) width_, (float) height_,
							  0.0f, 1.0f,
							  {0, 0},
							  {width_, height_}
					  }}
	);

	RenderPass::Create(rm_,
					   Stock::simpleForwardRendererRenderPass,
					   {
							   {
									   LoadOp::Clear,
									   StoreOp::Store,
									   GenericTextureFormat::R8G8B8A8_UNORM,
							   },
							   {
									   LoadOp::Clear,
									   StoreOp::DontCare,
									   GenericTextureFormat::D32_SFLOAT_S8_UINT,
									   LoadOp::Clear,
									   StoreOp::DontCare
							   }
					   },
					   {
							   {0.0f, 0.0f, 0.0f, 1.0f},
							   {0.0f, 255.0f, 0.0f, 0.0f},
					   }
	);

	RenderTarget::Create(rm_,
						 Stock::simpleForwardRendererRenderTarget,
						 rm_->openByName<RenderPassId>(Stock::simpleForwardRendererRenderPass),
						 {
								 {rm_->openByName<TextureId>(Stock::simpleForwardRendererColourRT)},
								 {rm_->openByName<TextureId>(Stock::simpleForwardRendererDepthStencilRT)},
						 },
						 {0, 0},
						 {width_, height_}
	);
	RasterisationState::Create(rm_,
							   Stock::simpleForwardRendererDefaultPipeline,
							   RasterisationStateFlags::DepthTestEnable | RasterisationStateFlags::DepthWriteEnable
	);

}

}