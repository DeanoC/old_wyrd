#include "meshmod.h"
#include "scenenode.h"
#include <stack>

namespace MeshMod {

void SceneNode::visitDescendents(Math::Matrix4x4 const& rootMatrix, std::function<void(SceneNode const&, Math::Matrix4x4 const&)> func ) const
{
	std::stack<MeshMod::SceneNode const*> nodeStack;
	std::stack<Math::Matrix4x4> matrixStack;

	nodeStack.push(this);
	matrixStack.push(rootMatrix);

	while (!nodeStack.empty())
	{
		auto const node = nodeStack.top();
		nodeStack.pop();
		auto const parentMatrix = matrixStack.top();
		matrixStack.pop();

		Math::Matrix4x4 local = node->transform.MakeMatrix();
		Math::Matrix4x4 world = local * parentMatrix;

		func(*node, world);

		for (auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i).get());
			matrixStack.push(world);
		}
	}
}

void SceneNode::mutateDescendents(Math::Matrix4x4 const& rootMatrix, std::function<void(SceneNode&, Math::Matrix4x4 const&)> func)
{
	std::stack<MeshMod::SceneNode*> nodeStack;
	std::stack<Math::Matrix4x4> matrixStack;

	nodeStack.push(this);
	matrixStack.push(rootMatrix);

	while (!nodeStack.empty())
	{
		auto const node = nodeStack.top();
		nodeStack.pop();
		auto const parentMatrix = matrixStack.top();
		matrixStack.pop();

		Math::Matrix4x4 local = node->transform.MakeMatrix();
		Math::Matrix4x4 world = local * parentMatrix;

		func(*node, world);

		for (auto i = 0u; i < node->getChildCount(); ++i)
		{
			nodeStack.push(node->getChild(i).get());
			matrixStack.push(world);
		}
	}
}


void SceneNode::visitObjects( std::function<void(ConstSceneObjectPtr)> func ) const
{
	for(auto obj : objects)
	{
		func(obj);
	}
}
void SceneNode::mutateObjects(std::function<void(SceneObjectPtr)> func)
{
	for (auto obj : objects)
	{
		func(obj);
	}
}

} // end namespace