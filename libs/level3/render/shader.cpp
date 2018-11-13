#include "core/core.h"
#include "core/quick_hash.h"
#include "resourcemanager/resourceman.h"
#include "resourcemanager/textresource.h"
#include "binny/writehelper.h"
#include "render/shader.h"
#include "shaderc/shaderc.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {
auto LanguageConverter(Render::ShaderSourceLanguage sourceLanguage_) -> shaderc_source_language
{
	using namespace Render;
	switch(sourceLanguage_)
	{
		case ShaderSourceLanguage::GLSL:
			return shaderc_source_language_glsl;
		case ShaderSourceLanguage::HLSL:
			return shaderc_source_language_hlsl;
		default:
			assert(false); // unknown source language
	}
}

auto TypeConverter(Render::ShaderType type_) -> shaderc_shader_kind
{
	using namespace Render;
	switch(type_)
	{
		case ShaderType::Vertex:
			return shaderc_vertex_shader;
		case ShaderType::TesselationControl:
			return shaderc_tess_control_shader;
		case ShaderType::TesselationEval:
			return shaderc_tess_evaluation_shader;
		case ShaderType::Geometry:
			return shaderc_geometry_shader;
		case ShaderType::Fragment:
			return shaderc_fragment_shader;
		case ShaderType::Compute:
			return shaderc_compute_shader;
		default:
			assert(false); // unknown shader type
	}
}

}
namespace Render {

// Compiles a shader to a SPIR-V binary. Returns the binary as
// a vector of 32-bit words.
std::vector<uint32_t> compile_file(const std::string& source_name,
								   shaderc_shader_kind kind,
								   const std::string& source)
{
	using namespace shaderc;
	Compiler compiler;
	CompileOptions options;

	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	SpvCompilationResult module =
			compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

	if(module.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cerr << module.GetErrorMessage();
		return std::vector<uint32_t>();
	}

	return {module.cbegin(), module.cend()};
}

auto SPIRVShader::RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void
{
	auto load = [](int stage, ResourceManager::ResolverInterface resolver_,
				   uint16_t majorVersion_, uint16_t minorVersion_, std::shared_ptr<void> ptr_) -> bool
	{
		assert(stage == 0);
		if(majorVersion_ != MajorVersion) return false;
		auto spirvShader = std::static_pointer_cast<SPIRVShader>(ptr_);
		auto[getRMFunc, resolverFunc, resourceNameFunc] = resolver_;

		resolverFunc(spirvShader->sourceHandle.base);

		return true;
	};

	auto destroy = [](int, void*)
	{
		return true;
	};
	auto changed = [](std::shared_ptr<ResourceBase const> ptr_)
	{
		return false;
	};
	auto save = [](std::shared_ptr<ResourceBase const> ptr_, ResourceManager::ISaver& writer_)
	{
		auto spirvShader = std::static_pointer_cast<SPIRVShader const>(ptr_);

		return true;
	};

	rm_.registerHandler(Id, {0, load, destroy}, changed, save);
}

auto SPIRVShader::Create(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		ResourceManager::TextResourceHandle sourceHandle_,
		ShaderSourceLanguage sourceLanguage_,
		ShaderType shaderType_,
		uint8_t flags_,
		std::vector<uint32_t> const& spirvCode_) -> SPIRVShaderHandle
{
	size_t const dataSize = sizeof(uint32_t) * spirvCode_.size();
	size_t const totalSize = Core::alignTo(sizeof(SPIRVShader) + dataSize, 8);
	auto* obj = (SPIRVShader*) malloc(totalSize);
	obj->sizeAndStageCount = totalSize;
	obj->sourceHandle = sourceHandle_;
	obj->sourceLanguage = sourceLanguage_;
	obj->shaderType = shaderType_;
	obj->codeCount = (uint32_t) spirvCode_.size();
	obj->flags = flags_;
	uint8_t* dataPtr = (uint8_t*) (obj + 1);
	std::memcpy(dataPtr, spirvCode_.data(), dataSize);

	rm_->placeInStorage(name_, *obj);
	free(obj);
	return rm_->openByName<Id>(name_);
}

auto SPIRVShader::Compile(
		std::shared_ptr<ResourceManager::ResourceMan> rm_,
		ResourceManager::ResourceNameView const& name_,
		ResourceManager::TextResourceHandle sourceHandle_,
		ShaderSourceLanguage sourceLanguage_,
		ShaderType shaderType_,
		uint8_t flags_) -> SPIRVShaderHandle
{
	using namespace shaderc;
	Compiler compiler;
	CompileOptions options;

	shaderc_source_language lang = LanguageConverter(sourceLanguage_);
	shaderc_shader_kind kind = TypeConverter(shaderType_);
	auto sourceName = std::string(name_.getResourceName());

	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	if(sourceLanguage_ == ShaderSourceLanguage::GLSL)
	{
		options.SetForcedVersionProfile(450, shaderc_profile_core);
	}
	options.SetSourceLanguage(lang);

	std::vector<uint32_t> spirvCode;
	bool shaderCompiledOkay;
	do
	{
		auto source = sourceHandle_.acquire<ResourceManager::TextResource>();

		// compiler non null terminated strings, so we have to count
		size_t const srcLen = strlen(source->getText());
		assert(srcLen <= source->getTextSize() - 1);

		auto module = compiler.CompileGlslToSpv(
				source->getText(),
				srcLen,
				kind,
				sourceName.c_str(),
				"main",
				options);

		shaderCompiledOkay = module.GetCompilationStatus() == shaderc_compilation_status_success;
		if(!shaderCompiledOkay)
		{
			LOG_S(WARNING) <<
						   "Shader compile fail:\n" << source->getText() << module.GetErrorMessage();
		} else
		{
			spirvCode.resize(module.cend() - module.cbegin());
			std::memcpy(spirvCode.data(), module.cbegin(), spirvCode.size() * sizeof(uint32_t));
		}
	} while(!shaderCompiledOkay);

	return Create(rm_, name_, sourceHandle_, sourceLanguage_,
				  shaderType_, flags_, spirvCode);
}


} // end namespace