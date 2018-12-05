#pragma once
#ifndef WYRD_MESHMODRENDERER_H
#define WYRD_MESHMODRENDERER_H

#include "core/core.h"
#include "math/vector_math.h"
#include "render/resources.h"
#include "resourcemanager/resourcehandle.h"
#include <vector>
#include <unordered_map>

namespace MeshMod { class Mesh; class SceneNode; }
namespace ResourceManager { class ResourceMan; }
namespace Render { struct Encoder; }

namespace MidRender {

enum class MeshIndex : uint32_t;
enum class SceneIndex : uint32_t;

class MeshModRenderer
{
public:

	struct RenderData
	{
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

	auto addMeshMod(std::shared_ptr<MeshMod::Mesh> const& mesh_, bool smoothColours = false) -> MeshIndex;
	auto addScene(std::shared_ptr<MeshMod::SceneNode> const& rootNode) -> SceneIndex;

	auto render(Math::mat4x4 const& rootMatrix_, SceneIndex index_, std::shared_ptr<Render::Encoder> const& encoder_) -> void;

protected:
	std::shared_ptr<ResourceManager::ResourceMan> rm;
	std::vector<RenderData> meshes;
	std::vector<SceneData> scenes;

	Render::BindingTableMemoryMapHandle memoryMapHandle;
	Render::BindingTableHandle bindingTableHandle;
	Render::VertexInputHandle vertexInputHandle;
	Render::RenderPipelineHandle renderPipelineHandle;
	Render::SPIRVShaderHandle vertexShaderHandle;
	Render::SPIRVShaderHandle fragmentShaderHandle;

};

}

#endif //WYRD_MESHMODRENDERER_H