#include "core/core.h"
#include "gfxtest.h"
#include "resourcemanager/resourceman.h"
#include "midrender/meshmodrenderer.h"
#include "meshmod/scenenode.h"
#include "meshmod/sceneobject.h"
#include "meshmod/mesh.h"
#include "meshops/shapes.h"
#include "meshops/gltf.h"
#include "render/encoder.h"

struct TacmapGltfTest : public GfxTest
{
	~TacmapGltfTest() final
	{

	}

	void init(ResourceManager::ResourceMan::Ptr const& rm_) final
	{
		rm = rm_;
		meshModRenderer.reset(new MidRender::MeshModRenderer());
		meshModRenderer->init(rm);

		rootScene = std::make_shared<MeshMod::SceneNode>();
		rootScene->transform.position = Math::vec3(0, 0, -40);

		std::shared_ptr sponza = MeshOps::Gltf::LoadAscii("D:/tacmap/NJR_RES_PRC_Residential_24x24_B.gltf");
		rootScene->addChild(sponza);

		sceneIndex = meshModRenderer->addScene(rootScene);
	}

	void tick(double deltaT_, std::shared_ptr<Render::Encoder>& encoder_) final
	{
		yrot += float(deltaT_ * 0.5);
		rootScene->transform.orientation = Math::rotate(
				Math::identity<Math::quat>(),
				yrot,
				Math::vec3(0, 1, 0));
		rootScene->transform.orientation = Math::rotate(
				rootScene->transform.orientation,
				yrot / 2.0f,
				Math::vec3(0, 0, 2));

		meshModRenderer->render(Math::identity<Math::mat4x4>(), sceneIndex, encoder_);
	}

	void finish() final
	{
		meshModRenderer->destroy();
		meshModRenderer.reset();
	}

	float yrot = 0.0f;

	ResourceManager::ResourceMan::Ptr rm;

	std::unique_ptr<MidRender::MeshModRenderer> meshModRenderer;
	MidRender::SceneIndex sceneIndex;
	std::shared_ptr<MeshMod::SceneNode> rootScene;
};

std::unique_ptr<GfxTest> CreateTacmapGltfTest()
{
	return std::move(std::make_unique<TacmapGltfTest>());
}
//
// Created by deano on 03/12/2018.
//

