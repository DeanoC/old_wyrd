#pragma once
#ifndef WYRD_MIDRENDER_STOCKS_H
#define WYRD_MIDRENDER_STOCKS_H

#include "core/core.h"
#include "resourcemanager/resourcename.h"

namespace ResourceManager { class ResourceMan; }

namespace MidRender {

struct Stocks
{
	static auto InitBasics(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void;

	static auto InitSimpleForwardRenderer(std::shared_ptr<ResourceManager::ResourceMan> const& rm_, uint32_t width_, uint32_t height_) -> void;

};

namespace Stock
{
// basic stocks
inline static auto defaultRenderPass = ResourceManager::ResourceNameView("mem$defaultRenderPass");
inline static auto singleOpaqueROPBlender = ResourceManager::ResourceNameView("mem$singleOpaqueROPBlender");
inline static auto singleOverROPBlender = ResourceManager::ResourceNameView("mem$singleOverROPBlender");
inline static auto singleDynamicViewport = ResourceManager::ResourceNameView("mem$singleDynamicViewport");
inline static auto defaultRasterState = ResourceManager::ResourceNameView("mem$defaultRasterState");

inline static auto blank4x4Tex = ResourceManager::ResourceNameView("mem$blankTex4x4");
inline static auto positionOnlyVertexInput = ResourceManager::ResourceNameView("mem$positionOnlyVertexInput");
inline static auto passthroughVertexShader = ResourceManager::ResourceNameView("mem$passthroughVertexShader");

// simple forward render stocks
inline static auto simpleForwardRendererColourRT = ResourceManager::ResourceNameView("mem$simpleForwardRendererColourRT");
inline static auto simpleForwardRendererDepthStencilRT = ResourceManager::ResourceNameView("mem$simpleForwardRendererDepthStencilRT");
inline static auto simpleForwardRendererViewport = ResourceManager::ResourceNameView("mem$simpleForwardRendererViewport");
inline static auto simpleForwardRendererRenderPass = ResourceManager::ResourceNameView("mem$simpleForwardRendererRenderPass");
inline static auto simpleForwardRendererRenderTarget = ResourceManager::ResourceNameView("mem$simpleForwardRendererRenderTarget");
inline static auto simpleForwardRendererDefaultPipeline = ResourceManager::ResourceNameView("mem$simpleForwardRendererDefaultPipeline");

}
}

#endif //WYRD_STOCKS_H
