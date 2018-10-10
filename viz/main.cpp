#define LOGURU_IMPLEMENTATION 1
#define CX_ERROR_IMPLEMENTATION 1
#include "core/core.h"
#include "common.h"
#include "entry/input.h"
#include "bx/file.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"
#include "meshops/basicmeshops.h"
#include "core/vector_math.h"
#include "core/quick_hash.h"
#include "binny/bundle.h"
#include "binny/bundlewriter.h"
#include "guizmo_math.h"
#include "meshmod/meshmod.h"
#include "meshmod/mesh.h"
#include "meshops/platonicsolids.h"
#include "meshops/gltf.h"
#include "meshops/convexhullcomputer.h"
#include "meshops/layeredtexture.h"
#include "enkiTS/src/TaskScheduler.h"
#include "Imgui-IGS-Snippet/ImguiWindowsFileIO.hpp"
#include "vectordisplay.h"
#include <stack>
#include <cstdio>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <array>
#include "playfield/hexfield.h"

struct VizPosColorUvVertex
{
	float m_x;
	float m_y;
	float m_z;
	float m_u;
	float m_v;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();
	}

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl VizPosColorUvVertex::ms_decl;
enki::TaskScheduler g_EnkiTS;

Math::Matrix4x4
EditTransform(float const *view, float const *proj, Math::Vector3 &position, Math::Vector3 &eulerAnglesInDegrees,
			  Math::Vector3 &scale)
{
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
	static bool uniformScale = true;

	if(ImGui::IsKeyPressed(90))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if(ImGui::IsKeyPressed(69))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if(ImGui::IsKeyPressed(82)) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	if(ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if(ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if(ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	ImGui::InputFloat3("Translate", position.data());
	ImGui::InputFloat3("euler angles", eulerAnglesInDegrees.data());

	ImGui::InputFloat3("Scale", scale.data());

	float const ex = Math::degreesToRadians(eulerAnglesInDegrees.x);
	float const ey = Math::degreesToRadians(eulerAnglesInDegrees.y);
	float const ez = Math::degreesToRadians(eulerAnglesInDegrees.z);

	if(mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if(ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if(ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	} else
	{
		if(ImGui::RadioButton("Uniform", !uniformScale))
			uniformScale = false;
		ImGui::SameLine();
		if(ImGui::RadioButton("Non uniform", uniformScale))
		{
			uniformScale = true;
			bool const xgy = (scale.x > scale.y);
			bool const xgz = (scale.x > scale.z);
			bool const ygz = (scale.y > scale.z);
			float uni = xgy ? (xgz ? scale.x : scale.z)
							: (ygz ? scale.y : scale.z);
			scale.x = scale.y = scale.z = uni;
		}
	}

	if(uniformScale)
	{
		bool const xey = (scale.x == scale.y);
		bool const xez = (scale.x == scale.z);
		float uni = xey ? (xez ? scale.x : scale.z)
						: (xez ? scale.y : scale.x);
		scale.x = scale.y = scale.z = uni;
	}

	ImGuiIO &io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	float mat[16];
	ImGuizmo::RecomposeMatrixFromComponents(position.data(), eulerAnglesInDegrees.data(), scale.data(), mat);
	ImGuizmo::Manipulate(view, proj, mCurrentGizmoOperation, mCurrentGizmoMode, mat, NULL, NULL);
	ImGuizmo::DecomposeMatrixToComponents(mat, position.data(), eulerAnglesInDegrees.data(), scale.data());


	return Math::Matrix4x4(mat);
}


struct RenderMesh
{
	size_t numPrims;
	bgfx::VertexBufferHandle vertexBuffer;
	bgfx::IndexBufferHandle indexBuffer;
};

class Viz : public entry::AppI
{
public:
	Viz(const char *_name, const char *_description)
			: entry::AppI(_name, _description)
	{}

	void init(int32_t _argc, const char *const *_argv, uint32_t _width, uint32_t _height) override;

	int shutdown() override;

	bool update() override;

	typedef std::unordered_map<MeshMod::Mesh const*, size_t> Mesh2MeshLookupTable;

	void uploadToGpu(MeshMod::SceneNodePtr const &rootNode, Mesh2MeshLookupTable &mesh2mesh);

	std::shared_ptr<RenderMesh> convertMesh(std::shared_ptr<MeshMod::Mesh> mesh, bool colourUsingPolygonNormal);

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	int64_t m_timeOffset;
	bx::DefaultAllocator defaultAllocator;

	MeshMod::SceneNodePtr rootScene;
	std::vector<std::string> recentFileList;
	bool fileIOVisible = false;
	enum class LoadState
	{
		None,
		WaitForMesh,
	} loadState = LoadState::None;

	Math::Vector3 rootPosition = Math::Vector3(0, 0, 0);
	Math::Vector3 rootHPBInDegrees = Math::Vector3(0, 0, 0);
	Math::Vector3 rootScale = Math::Vector3(1, 1, 1);

	bgfx::ProgramHandle basicProgram;
	VectorDisplay vectorDisplay;

	Mesh2MeshLookupTable mesh2mesh;
	std::vector<std::shared_ptr<RenderMesh>> renderMeshes;

	void remove50PercentPolygons();
	void replaceWithConvexHull();
	void createTetrahedron();
	void createCube();
	void createOctahedron();
	void createIcosahedron();

};

ENTRY_IMPLEMENT_MAIN(Viz, "Viz", "Viz for MeshMod.");

void Viz::init(int32_t _argc, const char *const *_argv, uint32_t _width, uint32_t _height)
{
	loguru::init(_argc, (char**)_argv);
	Args args(_argc, _argv);

	char log_path[PATH_MAX];
	loguru::suggest_log_path("~/loguru/", log_path, sizeof(log_path));
	loguru::add_file(log_path, loguru::FileMode::Truncate, loguru::Verbosity_MAX);
	loguru::add_file("everything.log", loguru::FileMode::Truncate, loguru::Verbosity_MAX);

	m_width = _width;
	m_height = _height;
	m_debug = BGFX_DEBUG_TEXT;
	m_reset = BGFX_RESET_VSYNC;

	bgfx::Init init;

	init.type = args.m_type;
	// shader compiler failing on metal at the moment
#if PLATFORM_OS == OSX
	init.type = bgfx::RendererType::OpenGL;
#endif
	init.vendorId = args.m_pciId;
	init.resolution.width = m_width;
	init.resolution.height = m_height;
	init.resolution.reset = m_reset;
	bgfx::init(init);

	g_EnkiTS.Initialize();

	// Enable debug text.
	bgfx::setDebug(m_debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	// Create vertex stream declaration.
	VizPosColorUvVertex::init();

	m_timeOffset = bx::getHPCounter();

	imguiCreate();

	// Setup style
	ImGui::StyleColorsClassic();

	//	MeshMod::SceneNodePtr testScene = MeshOps::Gltf::LoadAscii("models/glTF-Sample-Models-master/2.0/Avocado/glTF/Avocado.gltf"); 
	//	MeshMod::SceneNodePtr testScene = MeshOps::Gltf::LoadAscii("models/Cube/Cube.gltf");
	//	MeshOps::Gltf::SaveAscii(testScene, "test.gltf");

	basicProgram = loadProgram("vs_basic", "fs_basic");

	const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
	float texelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;
	bool originBottomLeft = bgfx::RendererType::OpenGL == renderer|| bgfx::RendererType::OpenGLES == renderer;
	vectorDisplay.init(originBottomLeft, texelHalf);
	vectorDisplay.setup(uint16_t(m_width), uint16_t(m_height));

	Playfield::BaseHexCoord<int8_t> a(10, 10);
	Playfield::BaseHexCoord<int8_t> b = (Playfield::BaseHexCoord<int8_t>)(Math::Vector2(100.f, 100.f));
	Playfield::BaseHexCoord<int8_t> c = Playfield::BaseHexCoord<int8_t>::round(Math::Vector2(101.5f, 102.5f));
//	Playfield::BaseHexCoord<int8_t> e(Playfield::PointyHexDirections::E);
}

int Viz::shutdown()
{
	imguiDestroy();

	vectorDisplay.teardown();

	// Shutdown bgfx.
	bgfx::shutdown();

	return 0;
}

bool Viz::update()
{
	if(!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
	{
		const uint8_t *chr = inputGetChar();

		imguiBeginFrame(m_mouseState.m_mx, m_mouseState.m_my,
						(m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
						| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
						| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0),
						m_mouseState.m_mz, uint16_t(m_width), uint16_t(m_height),
						chr ? (char) *chr : 0
		);

		// create a window and insert the inspector
		ImGui::SetNextWindowPos(
			ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(
			ImVec2(m_width / 5.0f, m_height / 4.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("Operations");

		std::string open_file;

		if(fileIOVisible)
		{
			fileIOVisible = true;
			std::vector<std::string> types;
			switch (loadState)
			{
			case LoadState::WaitForMesh: types.emplace_back("*.gltf"); break;
			default:break;
			}
			types.emplace_back("*.*");

			if(fileIOWindow(open_file, recentFileList, "Open", types, true, ImVec2(640, 300)))
			{
				fileIOVisible = false;
			}
		}

		if (!fileIOVisible)
		{
			switch (loadState)
			{
				case LoadState::None: break;
				case LoadState::WaitForMesh:
				{
					if (!open_file.empty())
					{
						recentFileList.push_back(open_file);
						rootScene = MeshOps::Gltf::LoadAscii(open_file);
						mesh2mesh.clear();
						uploadToGpu(rootScene, mesh2mesh);
					}
					loadState = LoadState::None;
					break;
				}
			}
		}

		if (ImGui::Button("Open"))
		{
			fileIOVisible = true;
			loadState = LoadState::WaitForMesh;
		}

		if(ImGui::Button("Remove 50% polygons"))
		{
			remove50PercentPolygons();
		}
		if (ImGui::Button("Replace with ConvexHull"))
		{
			replaceWithConvexHull();
		}

		if (ImGui::Button("Tetrahedron"))
		{
			createTetrahedron();
		}
		ImGui::SameLine();
		if (ImGui::Button("Octohedron"))
		{
			createOctahedron();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cube"))
		{
			createCube();
		}
		ImGui::SameLine();
		if (ImGui::Button("Isosehedron"))
		{
			createIcosahedron();
		}

		ImGui::End();

		float at[3] = {10.0f, 0.0f, 10.0f};
		float eye[3] = {10.0f, 25.0f, 10.0f};
		float up[3] = { 0.0f, 0.0f, 1.0f };

		float view[16];
		bx::mtxLookAt(view, eye, at, up);

		float proj[16];
		bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, view, proj);

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

		Math::Matrix4x4 rootMatrix = Math::IdentityMatrix();
		if(rootScene)
		{
			// create a window and insert the inspector
			ImGui::SetNextWindowPos(ImVec2(m_width - m_width / 5.0f - 10.0f, 400), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(600, 240), ImGuiCond_FirstUseEver);
			ImGui::Begin("Matrix Inspector");


			rootMatrix = EditTransform(view, proj, rootPosition, rootHPBInDegrees, rootScale);

			ImGui::End();
		}

		imguiEndFrame();

		float time = (float) ((bx::getHPCounter() - m_timeOffset) / double(bx::getHPFrequency()));

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		if(rootScene)
		{
			rootScene->visitDescendents(rootMatrix, [this](MeshMod::SceneNode const& node, Math::Matrix4x4 const& worldMat) {
				node.visitObjects([this, &worldMat](MeshMod::ConstSceneObjectPtr obj) {
					auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh const>(obj);
					if (mesh2mesh.find(mesh.get()) == mesh2mesh.end()) return;

					std::shared_ptr<RenderMesh> rm = renderMeshes[mesh2mesh[mesh.get()]];

					bgfx::setTransform(worldMat.data());

					// Set vertex and index buffer.
					bgfx::setVertexBuffer(0, rm->vertexBuffer);
					bgfx::setIndexBuffer(rm->indexBuffer);
					// Set render states.
					bgfx::setState(0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LESS
						| BGFX_STATE_CULL_CW
						| BGFX_STATE_MSAA
					);

					bgfx::submit(0, basicProgram);
				});
			});
		}

		vectorDisplay.beginFrame();


		vectorDisplay.endFrame();

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		return true;
	}

	return false;
}

void Viz::remove50PercentPolygons()
{
	std::stack<MeshMod::SceneNodePtr> nodeStack;
	if(rootScene)
	{
		nodeStack.push(rootScene);
	}

	static const uint32_t meshTypeHash = Core::QuickHash("Mesh");

	while(!nodeStack.empty())
	{
		auto const &node = nodeStack.top();
		nodeStack.pop();

		for(auto i = 0u; i < node->getObjectCount(); ++i)
		{
			auto const &obj = node->getObject(i);
			if(Core::QuickHash(obj->getType()) == meshTypeHash)
			{
				auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
				std::shared_ptr<RenderMesh> rm = renderMeshes[mesh2mesh[mesh.get()]];
				MeshMod::Polygons& polygons = mesh->getPolygons();
				for(auto j = 0u; j < polygons.getCount(); ++j)
				{
					if(j & 0x1) polygons.remove(j);
				}
				mesh->updateEditState(MeshMod::Mesh::TopologyEdits);
				mesh->updateFromEdits();

				renderMeshes[mesh2mesh[mesh.get()]] = convertMesh(mesh, false);
			}
		}
		for(auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i));
		}
	}
}

void Viz::replaceWithConvexHull()
{
	Math::Matrix4x4 rootMatrix = Math::IdentityMatrix();

	rootScene->mutateDescendents(rootMatrix, [this](MeshMod::SceneNode& node, Math::Matrix4x4 const& worldMat) {
		node.mutateObjects([this, &worldMat](MeshMod::SceneObjectPtr obj) {
			auto mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
			if (!mesh) return;
			MeshOps::ConvexHullComputer::generateInline(mesh);
			renderMeshes[mesh2mesh[mesh.get()]] = convertMesh(mesh, false);
		});
	});
}

void Viz::createTetrahedron()
{
	auto solid = MeshOps::PlatonicSolids::createTetrahedon();

	MeshOps::BasicMeshOps bops(solid);
	bops.triangulate();

	rootScene = std::make_shared<MeshMod::SceneNode>();
	rootScene->addObject(solid);

	mesh2mesh.clear();
	uploadToGpu(rootScene, mesh2mesh);
	MeshOps::Gltf::SaveAscii(rootScene, "D:\\meshdump\\tetrahedron.gltf");

}

void Viz::createOctahedron()
{
	auto solid = MeshOps::PlatonicSolids::createOctahedron();

	MeshOps::BasicMeshOps bops(solid);
	bops.triangulate();

	rootScene = std::make_shared<MeshMod::SceneNode>();
	rootScene->addObject(solid);

	mesh2mesh.clear();
	uploadToGpu(rootScene, mesh2mesh);
	MeshOps::Gltf::SaveAscii(rootScene, "D:\\meshdump\\octahedron.gltf");

}

void Viz::createCube()
{
	auto cube = MeshOps::PlatonicSolids::createCube();

	MeshOps::BasicMeshOps bops(cube);
	bops.triangulate();

	rootScene = std::make_shared<MeshMod::SceneNode>();
	rootScene->addObject(cube);

	mesh2mesh.clear();
	uploadToGpu(rootScene, mesh2mesh);

	MeshOps::Gltf::SaveAscii(rootScene, "D:\\meshdump\\cube.gltf");
}

void Viz::createIcosahedron()
{
	auto solid = MeshOps::PlatonicSolids::createIcosahedron();

	MeshOps::BasicMeshOps bops(solid);
	bops.triangulate();

	rootScene = std::make_shared<MeshMod::SceneNode>();
	rootScene->addObject(solid);

	mesh2mesh.clear();
	uploadToGpu(rootScene, mesh2mesh);

	MeshOps::Gltf::SaveAscii(rootScene, "D:\\meshdump\\icosahedron.gltf");

}

std::shared_ptr<RenderMesh> Viz::convertMesh(std::shared_ptr<MeshMod::Mesh> mesh, bool colourUsingPolygonNormal)
{
	using namespace MeshMod;

	if (colourUsingPolygonNormal)
	{
		MeshOps::BasicMeshOps bops(mesh);
		bops.computeFacePlaneEquations(false);
	}

	auto const& vertices = mesh->getVertices();
	auto const& polygons = mesh->getPolygons();
	if (polygons.getCount() == 0) return nullptr;

	std::vector<VizPosColorUvVertex> renderVertices;
	std::vector<uint32_t> indices;

	static const int maxVerticesPerFace = 20;
	for(auto iFace = 0u; iFace < polygons.getCount(); iFace++)
	{
		if(!polygons.isValid(iFace)) continue;

		VertexIndexContainer faceVertexIndices;
		polygons.getVertexIndices(iFace, faceVertexIndices);
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
			renderVertices[baseVertex + i].m_x = pos.x;
			renderVertices[baseVertex + i].m_y = pos.y;
			renderVertices[baseVertex + i].m_z = pos.z;
			renderVertices[baseVertex + i].m_u = 0.0f; // TODO Uvs
			renderVertices[baseVertex + i].m_v = 0.0f;

			if (colourUsingPolygonNormal)
			{
				auto const& planeEqs = polygons.getAttributes<PolygonData::PlaneEquations>();
				Math::Vector3 normal = planeEqs[iFace].planeEq.normal();

				uint8_t r = (uint8_t)(((normal.x + 1)*0.5f)*255.f);
				uint8_t g = (uint8_t)(((normal.y + 1)*0.5f)*255.f);
				uint8_t b = (uint8_t)(((normal.z + 1)*0.5f)*255.f);
				renderVertices[baseVertex + i].m_abgr = (0xFF << 24) |
														(b << 16) |
														(g << 8) |
														(r << 0);
			} else
			{
				renderVertices[baseVertex + i].m_abgr = colMap[iFace & 0x7];
			}
		}

		for(unsigned int iCoord = 2; iCoord < faceVertexIndices.size(); iCoord++)
		{
			indices.push_back((VertexIndex) (baseVertex + iCoord - 2));
			indices.push_back((VertexIndex) (baseVertex + iCoord - 1));
			indices.push_back((VertexIndex) (baseVertex + iCoord));
		}
	}

	bgfx::Memory const *bgv = bgfx::copy(renderVertices.data(), (uint32_t)renderVertices.size() * sizeof(VizPosColorUvVertex));
	bgfx::Memory const *bgi = bgfx::copy(indices.data(), (uint32_t) indices.size() * sizeof(uint32_t));

	std::shared_ptr<RenderMesh> renderMesh = std::make_shared<RenderMesh>();
	renderMesh->numPrims = indices.size() / 3;
	renderMesh->vertexBuffer = bgfx::createVertexBuffer(bgv, VizPosColorUvVertex::ms_decl, BGFX_BUFFER_NONE);
	renderMesh->indexBuffer = bgfx::createIndexBuffer(bgi, BGFX_BUFFER_INDEX32);

	return renderMesh;
}

void Viz::uploadToGpu(MeshMod::SceneNodePtr const &rootNode, Viz::Mesh2MeshLookupTable &mesh2mesh)
{
	static const uint32_t meshTypeHash = Core::QuickHash("Mesh");

	renderMeshes.clear();

	std::stack<MeshMod::SceneNodePtr> nodeStack;
	nodeStack.push(rootNode);

	while(!nodeStack.empty())
	{
		auto const &node = nodeStack.top();
		nodeStack.pop();
		for(auto i = 0u; i < node->getObjectCount(); ++i)
		{
			auto const &obj = node->getObject(i);
			if(Core::QuickHash(obj->getType()) == meshTypeHash)
			{
				MeshMod::MeshPtr mesh = std::dynamic_pointer_cast<MeshMod::Mesh>(obj);
				size_t index = renderMeshes.size();
				renderMeshes.resize(index + 1);
				mesh2mesh[mesh.get()] = index;
				renderMeshes[index] = convertMesh(mesh,true);
			}
		}
		for(auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i));
		}
	}
}