#pragma once
#ifndef WYRD_MESHMODRENDERER_H
#define WYRD_MESHMODRENDERER_H

#include "core/core.h"
#include "math/vector_math.h"
#include "render/resources.h"
#include "resourcemanager/resourcehandle.h"
#include <vector>
#include <unordered_map>
#include <array>

namespace MeshMod {
class Mesh;

class SceneNode;
}
namespace ResourceManager { class ResourceMan; }
namespace Render { struct Encoder; }

namespace MidRender {

enum class MeshIndex : uint32_t;
enum class SceneIndex : uint32_t;
struct IMeshModRenderStyle;

class MeshModRenderer
{
public:
	enum class RenderStyle
	{
		SolidConstant,
		SolidConstantWire,
		SolidNormalsFlat,
		SolidNormalsFlatWire,
		SolidNormalsPhong,
		SolidNormalsPhongWire,
		SolidDotFlat,
		SolidDotFlatWire,
		SolidDotPhong,
		SolidDotPhongWire,
	};

	static constexpr size_t RenderStylesCount = size_t(RenderStyle::SolidDotPhongWire) + 1;

	struct RenderData
	{
		RenderStyle style;
		std::array<float,4> colour;

		uint32_t numIndices;
		Render::BufferHandle vertexBufferHandle;
		Render::BufferHandle indexBufferHandle;
	};

	struct SceneData
	{
		using MeshMap = std::unordered_map<MeshMod::Mesh const*, MeshIndex>;
		MeshMap meshMap;
		std::shared_ptr<MeshMod::SceneNode> rootNode;
	};

	static constexpr MeshIndex InvalidMeshIndex{~0u};
	static constexpr SceneIndex InvalidSceneIndex{~0u};

	auto init(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void;
	auto destroy() -> void;

	auto addMeshMod(std::shared_ptr<MeshMod::Mesh> const& mesh_,
					RenderStyle style_ = RenderStyle::SolidNormalsFlatWire,
					std::array<float,4> const& colour_ = {0.5f,0.5f,0.5f,1.0f} ) -> MeshIndex;

	auto addScene(std::shared_ptr<MeshMod::SceneNode> const& rootNode,
				  RenderStyle style_ = RenderStyle::SolidNormalsFlatWire,
				  std::array<float,4> const& colour_ = {0.5f,0.5f,0.5f,1.0f}) -> SceneIndex;

	auto render(Math::mat4x4 const& rootMatrix_,
				SceneIndex index_,
				std::shared_ptr<Render::Encoder> const& encoder_) -> void;

protected:
	std::shared_ptr<ResourceManager::ResourceMan> rm;
	std::vector<RenderData> meshes;
	std::vector<SceneData> scenes;

	IMeshModRenderStyle* renderStyles[RenderStylesCount];
};

}

#endif //WYRD_MESHMODRENDERER_H