#include "core/core.h"
#include "core/quick_hash.h"
#include "midrender/meshmodrenderer.h"
#include "math/vector_math.h"
#include "meshmod/vertices.h"
#include "meshmod/polygons.h"
#include "meshmod/mesh.h"
#include "meshmod/scene.h"
#include "meshmod/vertexdata/normalvertex.h"
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


namespace MidRender {

struct IMeshModRenderStyle
{
	enum class FragmentStyle : int32_t
	{
		Constant = 0,
		Normal = 1,
		Dot = 2
	};
	virtual ~IMeshModRenderStyle() = default;

	virtual void init(
			std::shared_ptr<ResourceManager::ResourceMan> const& rm_,
			FragmentStyle style_,
			bool withWire_,
			bool phongNormals_) = 0;

	virtual void bind(
			MeshModRenderer::RenderData const& data_,
			std::shared_ptr<Render::Encoder> const& encoder_) = 0;

	virtual void pushConstants(
			MeshModRenderer::RenderData const& rd_,
			std::shared_ptr<Render::Encoder> const& encoder_,
			Math::mat4x4 const& worldMatrix_) = 0;

};

struct DefaultMeshModRenderStyle : public IMeshModRenderStyle
{
	void init(
			std::shared_ptr<ResourceManager::ResourceMan> const& rm_,
			FragmentStyle style_,
			bool withWire_,
			bool phongNormals_) final
	{
		using namespace Render;
		using namespace ResourceManager;
		using namespace std::literals;
		using namespace Core;
		using rnv = ResourceNameView;

		rm = rm_;

		bindingTableHandle = rm->openByName<BindingTableId>(rnv("mem$MMR_BindingTable0"));

		std::vector specializationConstants =
		{
				SpecializationConstant{
						{ withWire_ }, sizeof(int32_t),
						0,
						ShaderType::Vertex | ShaderType::Geometry | ShaderType::Fragment},

				SpecializationConstant{
						{ phongNormals_ }, sizeof(int32_t),
						1,
						ShaderType::Vertex | ShaderType::Geometry},

				SpecializationConstant{
						{(int32_t)style_}, sizeof(int32_t),
						1,
						ShaderType::Fragment},
		};

		std::vector shaders = {
				rm->openByName<SPIRVShaderId>(rnv("mem$MMR_all_VS")),
				rm->openByName<SPIRVShaderId>(rnv("mem$MMR_all_FS")),
		};

		if(withWire_ || !phongNormals_)
		{
			shaders.push_back(
					rm->openByName<SPIRVShaderId>(rnv("mem$MMR_all_GS"))
			);
		}

		renderPipelineHandle = RenderPipeline::Create(
				rm_,
				rnv(fmt::format("mem$MMR_RenderPipeline{}{}{}"s, (int) style_, withWire_, phongNormals_)),
				Topology::Triangles,
				RenderPipelineFlags::None,
				DynamicPipelineState::None,
				{
						rm->openByName<BindingTableMemoryMapId>(rnv("mem$MMR_BindingTableMemoryMap0")),
				},
				{
						{0,  sizeof(float) * 16, ShaderType::Vertex}, // world matrix
						{64, sizeof(float) * 4,  ShaderType::Fragment}, // colour
				},
				specializationConstants,
				shaders,
				rm->openByName<RasterisationStateId>(Stock::simpleForwardRasterState),
				rm->openByName<RenderPassId>(Stock::simpleForwardRenderPass),
				rm->openByName<ROPBlenderId>(Stock::singleOpaqueROPBlender),
				rm->openByName<VertexInputId>(rnv("mem$MMR_VertexFormat")),
				rm->openByName<ViewportId>(Stock::simpleForwardViewport)
		);

	};

	void bind(
			MeshModRenderer::RenderData const& data_,
			std::shared_ptr<Render::Encoder> const& encoder_) final
	{
		using namespace Render;
		using namespace ResourceManager;

		auto renderEncoder = encoder_->asRenderEncoder();
		auto renderPipeline = renderPipelineHandle.acquire<RenderPipeline>();
		auto bindingTable = bindingTableHandle.acquire<BindingTable>();

		renderEncoder->bind(renderPipeline, bindingTable);
	}

	void pushConstants(
			MeshModRenderer::RenderData const& rd_,
			std::shared_ptr<Render::Encoder> const& encoder_,
			Math::mat4x4 const& worldMatrix_) final
	{
		using namespace Render;
		using namespace ResourceManager;
		auto renderEncoder = encoder_->asRenderEncoder();
		auto renderPipeline = renderPipelineHandle.acquire<RenderPipeline>();

		renderEncoder->pushConstants(
				renderPipeline,
				PushConstantRange{0, sizeof(float) * 16, ShaderType::Vertex},
				&worldMatrix_);

		renderEncoder->pushConstants(
				renderPipeline,
				PushConstantRange{64, sizeof(float) * 4, ShaderType::Fragment},
				rd_.colour.data());
	}

	std::shared_ptr<ResourceManager::ResourceMan> rm;

	Render::RenderPipelineHandle renderPipelineHandle;
	Render::BindingTableHandle bindingTableHandle;
};

auto MeshModRenderer::init(std::shared_ptr<ResourceManager::ResourceMan> const& rm_) -> void
{
	using namespace Render;
	using namespace ResourceManager;
	using namespace Core;
	using namespace std::literals;
	using rnv = ResourceNameView;

	rm = rm_;

	TextResource::CreateFromFile(
			rm,
			rnv("mem$MMR_all_VS_Text"),
			"text/shaders/meshmodrenderer/all_VS.glsl");

	TextResource::CreateFromFile(
			rm,
			rnv("mem$MMR_all_GS_Text"),
			"text/shaders/meshmodrenderer/all_GS.glsl");

	TextResource::CreateFromFile(
			rm,
			rnv("mem$MMR_all_FS_Text"),
			"text/shaders/meshmodrenderer/all_FS.glsl");

	SPIRVShader::Compile(
			rm, rnv("mem$MMR_all_VS"),
			{rm->openByName<TextResourceId>(rnv("mem$MMR_all_VS_Text"))},
			ShaderSourceLanguage::GLSL, ShaderType::Vertex, 0);

	SPIRVShader::Compile(
			rm, rnv("mem$MMR_all_GS"),
			{rm->openByName<TextResourceId>(rnv("mem$MMR_all_GS_Text"))},
			ShaderSourceLanguage::GLSL, ShaderType::Geometry, 0);

	SPIRVShader::Compile(
			rm, ResourceNameView("mem$MMR_all_FS"),
			{rm->openByName<TextResourceId>(rnv("mem$MMR_all_FS_Text"))},
			ShaderSourceLanguage::GLSL, ShaderType::Fragment, 0);


	VertexInput::Create(
			rm, rnv("mem$MMR_VertexFormat"),
			{
					{VertexInputLocation::Position, VertexInputType::Float3},
					{VertexInputLocation::Normal,   VertexInputType::Float3}
			});

	auto bindingTableMemMap0 = BindingTableMemoryMap::Create(
			rm,
			rnv("mem$MMR_BindingTableMemoryMap0"),
			{
					{BindingTableType::Buffer, 1, ShaderType::Vertex | ShaderType::Geometry}
			}
	);

	auto bindingTable0Handle = BindingTable::Create(
			rm_,
			ResourceNameView("mem$MMR_BindingTable0"),
			{
					{bindingTableMemMap0}
			}
	);

	bindingTable0Handle.acquire<BindingTable>()->update(
			0, 0,
			rm->openByName<BufferId>(Stock::simpleForwardGlobalBuffer));

	for(auto i = 0u; i < RenderStylesCount; ++i)
	{
		bool const wire = i & 0x1;
		bool const phong = (i >=2) && ((i-2) & 0x2);
		IMeshModRenderStyle::FragmentStyle style =
				(i < 2) ? 	IMeshModRenderStyle::FragmentStyle::Constant :
				(i < 6) ? 	IMeshModRenderStyle::FragmentStyle::Normal :
							IMeshModRenderStyle::FragmentStyle::Dot;
		renderStyles[i] = new DefaultMeshModRenderStyle();
		renderStyles[i]->init(rm, style, wire, phong);
	}

}

auto MeshModRenderer::destroy() -> void
{
	for(auto i = 0u; i < RenderStylesCount; ++i)
	{
		delete renderStyles[i];
		renderStyles[i] = nullptr;
	}
}


auto MeshModRenderer::addMeshMod(
		std::shared_ptr<MeshMod::Mesh> const& mesh_,
		RenderStyle style_,
		std::array<float, 4> const& colour_) -> MeshIndex
{
	using namespace MeshMod;

	auto const& vertices = mesh_->getVertices();
	auto const& polygons = mesh_->getPolygons();
	if(polygons.getCount() == 0) return InvalidMeshIndex;

	MeshOps::BasicMeshOps::triangulate(mesh_);
	mesh_->getVertices().removeAllSimilarPositions();
	mesh_->updateFromEdits();

	MeshOps::BasicMeshOps::computeVertexNormalsEx(mesh_, false);

	struct PosNormalVertex
	{
		float x, y, z;
		float nx, ny, nz;
	};
	using VertexContainer = std::vector<PosNormalVertex>;
	using IndexContainer = std::vector<uint32_t>;
	VertexContainer renderVertices;
	IndexContainer indices;

	VertexIndexContainer faceVertexIndices(3);
	size_t baseVertex = renderVertices.size();
	renderVertices.resize(baseVertex + vertices.getCount());
	auto& rv = renderVertices;
	auto const& normals = vertices.getAttribute<VertexData::Normals>();

	for(auto i = 0u; i < vertices.getCount(); i++)
	{
		VertexIndex vertexIndex = VertexIndex(i);
		VertexData::Position const pos = vertices.position(vertexIndex);
		Math::vec3 const normal = normals.at(vertexIndex).getVec3();
		rv[baseVertex + i].x = pos.x;
		rv[baseVertex + i].y = pos.y;
		rv[baseVertex + i].z = pos.z;
		rv[baseVertex + i].nx = normal.x;
		rv[baseVertex + i].ny = normal.y;
		rv[baseVertex + i].nz = normal.z;
	}
	indices.resize(polygons.getCount() * 3);
	for(auto i = 0u; i < polygons.getCount(); i++)
	{
		auto polygonIndex = PolygonIndex(i);
		if(!polygons.isValid(polygonIndex)) continue;

		faceVertexIndices.clear();
		polygons.getVertexIndices(polygonIndex, faceVertexIndices);

		indices[i * 3 + 0] = (uint32_t) faceVertexIndices[0];
		indices[i * 3 + 1] = (uint32_t) faceVertexIndices[1];

		indices[i * 3 + 2] = (uint32_t) faceVertexIndices[2];
	}

	using namespace Render;
	using namespace std::literals;
	using namespace Core;

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
	rdata.style = style_;
	rdata.colour = colour_;

	uint32_t index = (uint32_t) meshes.size();
	meshes.push_back(rdata);
	return MeshIndex(index);
}

auto MeshModRenderer::addScene(
		std::shared_ptr<MeshMod::SceneNode> const& rootNode,
		RenderStyle style_,
		std::array<float, 4> const& colour_) -> SceneIndex
{
	using namespace std::literals;
	static constexpr uint32_t meshTypeHash = Core::QuickHash("Mesh"sv);

	SceneData sceneData;
	sceneData.rootNode = rootNode;

	std::stack<std::shared_ptr<MeshMod::SceneNode>> nodeStack;
	nodeStack.push(rootNode);

	while(!nodeStack.empty())
	{
		auto const node = nodeStack.top();
		nodeStack.pop();
		for(auto i = 0u; i < node->getObjectCount(); ++i)
		{
			auto const& obj = node->getObject(i);
			if(Core::QuickHash(obj->getType()) == meshTypeHash)
			{
				std::shared_ptr<MeshMod::Mesh> mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
				sceneData.meshMap[mesh.get()] = addMeshMod(mesh, style_, colour_);
			}
		}
		for(auto j = 0u; j < node->getChildCount(); ++j)
		{
			nodeStack.push(node->getChild(j));
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

	RenderStyle currentStyle = RenderStyle(~0);

	auto visitor = [this, &scene, &encoder_, &currentStyle](MeshMod::SceneNode const& node,
															Math::mat4x4 const& worldMat)
	{
		node.visitObjects(
				[this, &worldMat, &scene, &encoder_, &currentStyle](
						std::shared_ptr<MeshMod::SceneObject const> obj)
				{
					auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh const>(obj);
					auto meshIt = scene.meshMap.find(mesh.get());
					if(meshIt == scene.meshMap.end()) return;
					if(meshIt->second == InvalidMeshIndex) return;

					auto const& rd = meshes[size_t(meshIt->second)];

					auto vertexBuffer = rd.vertexBufferHandle.acquire<Render::Buffer>();
					auto indexBuffer = rd.indexBufferHandle.acquire<Render::Buffer>();

					if(currentStyle != rd.style)
					{
						renderStyles[size_t(rd.style)]->bind(rd, encoder_);
						currentStyle = rd.style;
					}
					renderStyles[size_t(rd.style)]->pushConstants(
							rd,
							encoder_,
							worldMat);

					auto renderEncoder = encoder_->asRenderEncoder();
					renderEncoder->bindVertexBuffer(vertexBuffer);
					renderEncoder->bindIndexBuffer(indexBuffer, 32);
					renderEncoder->drawIndexed(rd.numIndices);
				});
	};

	rootScene->visitDescendents(rootMatrix, visitor);
}

} // end namespace