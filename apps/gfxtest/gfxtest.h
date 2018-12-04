#pragma once
#ifndef WYRD_GFXTEST_H
#define WYRD_GFXTEST_H

#include "core/core.h"

namespace ResourceManager{ class ResourceMan; }
namespace Render { struct Encoder; }
struct GfxTest
{
	virtual ~GfxTest() = default;
	virtual auto init(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void = 0;
	virtual auto tick(double deltaT_, std::shared_ptr<Render::Encoder>& encoder_) -> void = 0;
	virtual auto finish() -> void = 0;

};

std::unique_ptr<GfxTest> CreateBasicMeshTest();
std::unique_ptr<GfxTest> CreateDuckGltfTest();
std::unique_ptr<GfxTest> CreateSponzaGltfTest();
std::unique_ptr<GfxTest> CreateTacmapGltfTest();

#endif //WYRD_GFXTEST_H
