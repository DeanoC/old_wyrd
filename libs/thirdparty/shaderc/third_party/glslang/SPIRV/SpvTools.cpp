//
// Copyright (C) 2014-2016 LunarG, Inc.
// Copyright (C) 2018 Google, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

//
// Call into SPIRV-Tools to disassemble, validate, and optimize.
//

#if ENABLE_OPT

#include <cstdio>
#include <iostream>

#include "SpvTools.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/libspirv.h"

namespace glslang {

// Translate glslang's view of target versioning to what SPIRV-Tools uses.
spv_target_env MapToSpirvToolsEnv(const SpvVersion& spvVersion, spv::SpvBuildLogger* logger)
{
	switch(spvVersion.vulkan)
	{
		case glslang::EShTargetVulkan_1_0:
			return spv_target_env::SPV_ENV_VULKAN_1_0;
		case glslang::EShTargetVulkan_1_1:
			return spv_target_env::SPV_ENV_VULKAN_1_1;
		default:
			break;
	}

	if(spvVersion.openGl > 0)
		return spv_target_env::SPV_ENV_OPENGL_4_5;

	logger->missingFunctionality("Target version for SPIRV-Tools validator");
	return spv_target_env::SPV_ENV_UNIVERSAL_1_0;
}


// Use the SPIRV-Tools disassembler to print SPIR-V.
void SpirvToolsDisassemble(std::ostream& out, const std::vector<unsigned int>& spirv)
{
	// disassemble
	spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_3);
	spv_text text;
	spv_diagnostic diagnostic = nullptr;
	spvBinaryToText(context, spirv.data(), spirv.size(),
					SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_INDENT,
					&text, &diagnostic);

	// dump
	if(diagnostic == nullptr)
		out << text->str;
	else
		spvDiagnosticPrint(diagnostic);

	// teardown
	spvDiagnosticDestroy(diagnostic);
	spvContextDestroy(context);
}

// Apply the SPIRV-Tools validator to generated SPIR-V.
void SpirvToolsValidate(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
						spv::SpvBuildLogger* logger)
{
	// validate
	spv_context context = spvContextCreate(MapToSpirvToolsEnv(intermediate.getSpv(), logger));
	spv_const_binary_t binary = {spirv.data(), spirv.size()};
	spv_diagnostic diagnostic = nullptr;
	spv_validator_options options = spvValidatorOptionsCreate();
	spvValidatorOptionsSetRelaxBlockLayout(options, intermediate.usingHlslOffsets());
	spvValidateWithOptions(context, options, &binary, &diagnostic);

	// report
	if(diagnostic != nullptr)
	{
		logger->error("SPIRV-Tools Validation Errors");
		logger->error(diagnostic->error);
	}

	// tear down
	spvValidatorOptionsDestroy(options);
	spvDiagnosticDestroy(diagnostic);
	spvContextDestroy(context);
}

// Apply the SPIRV-Tools optimizer to generated SPIR-V, for the purpose of
// legalizing HLSL SPIR-V.
void SpirvToolsLegalize(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
						spv::SpvBuildLogger* logger, const SpvOptions* options)
{
	spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;

	spvtools::Optimizer optimizer(target_env);
	optimizer.SetMessageConsumer(
			[](spv_message_level_t level, const char* source, const spv_position_t& position, const char* message)
			{
				auto& out = std::cerr;
				switch(level)
				{
					case SPV_MSG_FATAL:
					case SPV_MSG_INTERNAL_ERROR:
					case SPV_MSG_ERROR:
						out << "error: ";
						break;
					case SPV_MSG_WARNING:
						out << "warning: ";
						break;
					case SPV_MSG_INFO:
					case SPV_MSG_DEBUG:
						out << "info: ";
						break;
					default:
						break;
				}
				if(source)
				{
					out << source << ":";
				}
				out << position.line << ":" << position.column << ":" << position.index << ":";
				if(message)
				{
					out << " " << message;
				}
				out << std::endl;
			});

	optimizer.RegisterPass(spvtools::CreateMergeReturnPass());
	optimizer.RegisterPass(spvtools::CreateInlineExhaustivePass());
	optimizer.RegisterPass(spvtools::CreateEliminateDeadFunctionsPass());
	optimizer.RegisterPass(spvtools::CreateScalarReplacementPass());
	optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
	optimizer.RegisterPass(spvtools::CreateLocalSingleBlockLoadStoreElimPass());
	optimizer.RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
	optimizer.RegisterPass(spvtools::CreateSimplificationPass());
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateVectorDCEPass());
	optimizer.RegisterPass(spvtools::CreateDeadInsertElimPass());
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
	optimizer.RegisterPass(spvtools::CreateBlockMergePass());
	optimizer.RegisterPass(spvtools::CreateLocalMultiStoreElimPass());
	optimizer.RegisterPass(spvtools::CreateIfConversionPass());
	optimizer.RegisterPass(spvtools::CreateSimplificationPass());
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateVectorDCEPass());
	optimizer.RegisterPass(spvtools::CreateDeadInsertElimPass());
	if(options->optimizeSize)
	{
		optimizer.RegisterPass(spvtools::CreateRedundancyEliminationPass());
		// TODO(greg-lunarg): Add this when AMD driver issues are resolved
		// optimizer.RegisterPass(CreateCommonUniformElimPass());
	}
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateCFGCleanupPass());

	optimizer.Run(spirv.data(), spirv.size(), &spirv, spvtools::ValidatorOptions(), true);
}

}; // end namespace glslang

#endif
