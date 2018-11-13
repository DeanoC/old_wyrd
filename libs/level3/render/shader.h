#pragma once
#ifndef WYRD_RENDER_SHADER_H
#define WYRD_RENDER_SHADER_H

#include "core/core.h"
#include "core/utils.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resourceid.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourcehandle.h"
#include "resourcemanager/textresource.h"

namespace ResourceManager {
class ResourceMan;
}

namespace Render {

struct alignas(8) SPIRVShader : public ResourceManager::Resource<SPIRVShaderId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			ResourceManager::TextResourceHandle sourceHandle_,
			ShaderSourceLanguage sourceLanguage_,
			ShaderType shaderType_,
			uint8_t flags_,
			std::vector<uint32_t> const& spirvCode_
	) -> SPIRVShaderHandle;

	static auto Compile(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			ResourceManager::TextResourceHandle sourceHandle_,
			ShaderSourceLanguage sourceLanguage_,
			ShaderType shaderType_,
			uint8_t flags_
	) -> SPIRVShaderHandle;

	static constexpr uint8_t DebugFlag = Core::Bit(0);

	uint32_t* getByteCode() { return (uint32_t*) (this + 1); }

	ResourceManager::TextResourceHandle sourceHandle;
	ShaderSourceLanguage sourceLanguage;
	ShaderType shaderType;
	uint8_t flags;
	uint8_t padd[1];
	uint32_t codeCount; // no of 32 bit tokens
};

}

#endif //WYRD_RENDER_SHADER_H
