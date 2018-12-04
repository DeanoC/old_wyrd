#include "core/core.h"
#include "core/quick_hash.h"
#include "midrender/meshmodrenderer.h"
#include "math/vector_math.h"
#include "meshmod/vertices.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/scene.h"
#include "meshops/basicmeshops.h"
#include "resourcemanager/textresource.h"
#include "resourcemanager/resourceman.h"
#include "render/bindingtable.h"
#include "render/buffer.h"
#include "render/encoder.h"
#include "render/pipeline.h"
#include "render/shader.h"
#include "render/vertexinput.h"
#include "midrender/stocks.h"
#include "fmt/format.h"
#include <stack>

namespace {

}; // end anon namespace

namespace MidRender {

auto MeshModRenderer::init(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void
{
	using namespace Render;
	using namespace ResourceManager;
	using namespace Core::bitmask;

	rm = rm_;

	TextResource::CreateFromFile(rm, ResourceNameView("mem$MMR_VertexShaderText"), "text/shaders/meshmodrenderer_vert.glsl");
	TextResource::CreateFromFile(rm, ResourceNameView("mem$MMR_FragmentShaderText"), "text/shaders/meshmodrenderer_frag.glsl");

	vertexShaderHandle = SPIRVShader::Compile(rm,
						 ResourceNameView("mem$MMR_VertexShader"),
						 {rm->openByName<TextResourceId>(ResourceNameView("mem$MMR_VertexShaderText"))},
						 ShaderSourceLanguage::GLSL,
						 ShaderType::Vertex,
						 0);

	fragmentShaderHandle = SPIRVShader::Compile(rm,
						 ResourceNameView("mem$MMR_FragmentShader"),
						 {rm->openByName<TextResourceId>(ResourceNameView("mem$MMR_FragmentShaderText"))},
						 ShaderSourceLanguage::GLSL,
						 ShaderType::Fragment,
						 0);

	memoryMapHandle = BindingTableMemoryMap::Create(
			rm_,
			ResourceNameView("mem$MMR_BindingTableMemoryMap"),
			{
				{BindingTableType::Buffer, 1, ShaderType::Vertex }
			}
	);

	bindingTableHandle = BindingTable::Create(
			rm_,
			ResourceNameView("mem$MMR_BindingTable"),
			{
				{ memoryMapHandle }
			}
	);
	vertexInputHandle = VertexInput::Create(
			rm_,
			ResourceNameView("mem$MMR_VertexFormat"),
			{
					{VertexInputLocation(Position), VertexInputType::Float3},
					{VertexInputLocation(1), VertexInputType::Byte4}
			});

	renderPipelineHandle = RenderPipeline::Create(
			rm_,
			ResourceNameView("mem$MMR_RenderPipeline"),
			Topology::Triangles,
			RenderPipelineFlags::None,
			DynamicPipelineState::None,
			{ memoryMapHandle },
			{
					{0, sizeof(float)*16, ShaderType::Vertex} // world matrix
			},
			{
					vertexShaderHandle,
					fragmentShaderHandle
			},
			rm->openByName<RasterisationStateId>(Stock::simpleForwardRasterState),
			rm->openByName<RenderPassId>(Stock::simpleForwardRenderPass),
			rm->openByName<ROPBlenderId>(Stock::singleOpaqueROPBlender),
			vertexInputHandle,
			rm->openByName<ViewportId>(Stock::simpleForwardViewport)
	);

	bindingTableHandle.acquire<BindingTable>()->update(0, 0,
		rm->openByName<BufferId>(Stock::simpleForwardGlobalBuffer));
}

auto MeshModRenderer::destroy() -> void
{
}


auto MeshModRenderer::addMeshMod(std::shared_ptr<MeshMod::Mesh> const& mesh_,
								 bool colourUsingPolygonNormal_) -> MeshIndex
{
	using namespace MeshMod;

	if(colourUsingPolygonNormal_)
	{
		MeshOps::BasicMeshOps::computeFacePlaneEquations(mesh_, false);
	}
	MeshOps::BasicMeshOps::triangulate(mesh_);

	auto const& vertices = mesh_->getVertices();
	auto const& polygons = mesh_->getPolygons();
	if(polygons.getCount() == 0) return InvalidMeshIndex;

	struct PosColorVertex
	{
		float x, y, z;
		uint32_t argb;
	};
	using VertexContainer = std::vector<PosColorVertex>;
	using IndexContainer = std::vector<uint32_t>;
	VertexContainer renderVertices;
	IndexContainer indices;

	static const int maxVerticesPerFace = 20;
	for(auto iFace = 0u; iFace < polygons.getCount(); iFace++)
	{
		auto polygonIndex = PolygonIndex(iFace);
		if(!polygons.isValid(polygonIndex)) continue;

		VertexIndexContainer faceVertexIndices;
		polygons.getVertexIndices(polygonIndex, faceVertexIndices);
		assert(faceVertexIndices.size() < maxVerticesPerFace);

		size_t baseVertex = renderVertices.size();
		renderVertices.resize(baseVertex + faceVertexIndices.size());

		static const uint32_t colMap[] =
		{
				0x80FF0000,
				0x8000FF00,
				0x800000FF,
				0x8000FFFF,
				0x80FF00FF,
				0x80FFFF00,
				0x808000FF,
				0x80008080,
		};

		for(auto i = 0u; i < faceVertexIndices.size(); ++i)
		{
			VertexData::Position const pos = vertices.position(faceVertexIndices[i]);
			renderVertices[baseVertex + i].x = pos.x;
			renderVertices[baseVertex + i].y = pos.y;
			renderVertices[baseVertex + i].z = pos.z;

			if(colourUsingPolygonNormal_)
			{
				auto const& planeEqs = polygons.getAttribute<PolygonData::PlaneEquations>();
				Math::vec3 normal = planeEqs[polygonIndex].planeEq.normal();

				uint8_t a = 0xFF;
				uint8_t r = (uint8_t) (((normal.x + 1) * 0.5f) * 255.f);
				uint8_t g = (uint8_t) (((normal.y + 1) * 0.5f) * 255.f);
				uint8_t b = (uint8_t) (((normal.z + 1) * 0.5f) * 255.f);
				renderVertices[baseVertex + i].argb = (a << 24) |
													  (r << 16) |
													  (g << 8) |
													  (b << 0);
			} else
			{
				renderVertices[baseVertex + i].argb = colMap[iFace & 0x7];
			}
		}

		for(unsigned int iCoord = 2; iCoord < faceVertexIndices.size(); iCoord++)
		{
			indices.push_back((uint32_t) (baseVertex + iCoord - 2));
			indices.push_back((uint32_t) (baseVertex + iCoord - 1));
			indices.push_back((uint32_t) (baseVertex + iCoord));
		}
	}

	using namespace Render;
	using namespace std::literals;
	using namespace Core::bitmask;

	std::string iname = fmt::format("mem$MMR_IndexBuffer_{}_{}"s, mesh_->getName(), (uint32_t) meshes.size());
	std::string vname = fmt::format("mem$MMR_VertexBuffer_{}_{}"s, mesh_->getName(), (uint32_t) meshes.size());
	RenderData rdata{};
	rdata.numIndices = (uint32_t) indices.size();
	rdata.indexBufferHandle = Buffer::Create(
			rm,
			ResourceManager::ResourceNameView(iname),
			Buffer::FromUsage(Usage::DMADst | Usage::IndexRead),
			indices.size() * sizeof(IndexContainer::value_type),
			indices.data()
	);
	rdata.vertexBufferHandle = Buffer::Create(
			rm,
			ResourceManager::ResourceNameView(vname),
			Buffer::FromUsage(Usage::DMADst | Usage::VertexRead),
			renderVertices.size() * sizeof(VertexContainer::value_type),
			renderVertices.data()
	);

	uint32_t index = (uint32_t) meshes.size();
	meshes.push_back(rdata);
	return MeshIndex(index);
}

auto MeshModRenderer::addScene(std::shared_ptr<MeshMod::SceneNode> const& rootNode) -> SceneIndex
{
	using namespace std::literals;
	static constexpr uint32_t meshTypeHash = Core::QuickHash("Mesh"sv);

	SceneData sceneData;
	sceneData.rootNode = rootNode;

	std::stack<std::shared_ptr<MeshMod::SceneNode>> nodeStack;
	nodeStack.push(rootNode);


	while(!nodeStack.empty())
	{
		auto const& node = nodeStack.top();
		nodeStack.pop();
		for(auto i = 0u; i < node->getObjectCount(); ++i)
		{
			auto const& obj = node->getObject(i);
			if(Core::QuickHash(obj->getType()) == meshTypeHash)
			{
				std::shared_ptr<MeshMod::Mesh> mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
				sceneData.meshMap[mesh.get()] = addMeshMod(mesh, true);
			}
		}
		for(auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i));
		}
	}

	uint32_t index = (uint32_t) scenes.size();
	scenes.push_back(sceneData);
	return SceneIndex(index);

}

auto MeshModRenderer::render(
	Math::mat4x4 const& rootMatrix_,
	SceneIndex index_,
	std::shared_ptr<Render::Encoder> const& encoder_) -> void
{
	using namespace Render;
	if(index_ == InvalidSceneIndex) return;

	auto const& scene = scenes.at(size_t(index_));

	auto const& rootScene = scene.rootNode;
	auto rootMatrix = rootMatrix_;

	auto renderEncoder = encoder_->asRenderEncoder();
	auto renderPipeline = renderPipelineHandle.acquire<RenderPipeline>();
	auto bindingTable = bindingTableHandle.acquire<BindingTable>();

	renderEncoder->bind(renderPipeline, bindingTable);

	auto visitor = [this, &scene, renderEncoder, renderPipeline](MeshMod::SceneNode const& node, Math::mat4x4 const& worldMat)
	{
		node.visitObjects(
				[this, &worldMat, &scene, renderEncoder, renderPipeline](std::shared_ptr<MeshMod::SceneObject const> obj)
				{
					auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh const>(obj);
					auto meshIt = scene.meshMap.find(mesh.get());
					if(meshIt == scene.meshMap.end()) return;

					auto const& rd = meshes[size_t(meshIt->second)];

					auto vertexBuffer = rd.vertexBufferHandle.acquire<Render::Buffer>();
					auto indexBuffer = rd.indexBufferHandle.acquire<Render::Buffer>();
					renderEncoder->pushConstants(
							renderPipeline,
							PushConstantRange{0, sizeof(float)*16, ShaderType::Vertex},
							&worldMat);
					renderEncoder->bindVertexBuffer(vertexBuffer);
//					renderEncoder->bindIndexBuffer(indexBuffer, 32);
//					renderEncoder->drawIndexed(rd.numIndices);
					renderEncoder->draw(rd.numIndices);
				});
	};

	rootScene->visitDescendents(rootMatrix, visitor);
}

} // end namespace