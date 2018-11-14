#pragma once
#ifndef WYRD_RENDER_VERTEXINPUT_H
#define WYRD_RENDER_VERTEXINPUT_H

#include "core/core.h"
#include "render/resources.h"
#include "render/types.h"
#include "resourcemanager/resource.h"

namespace ResourceManager {
class ResourceMan;

struct ResourceNameView;
}

namespace Render {

// old skool but simple vertex input.
// assumes packed single stream all vertex (no instance) data
// TODO add a modern one or just use arbitary fetch for complex cases
// these names are just suggestion, the actual location etc. is whatever binding
// you choose, so feel free to ignore tham an use any old uint8 that fits your
// shaders..
enum VertexInputLocation : uint8_t
{
	Position = 0,
	Normal,
	Binormal,
	Tangent,
	Texcoord0,
	Texcoord1,
	Texcoord2,
	Texcoord3,
	Colour0,
	Colour1
};

enum class VertexInputType : uint8_t
{
	Float1,
	Float2,
	Float3,
	Float4,
	Byte4, // abgr unorm
};

struct alignas(8) VertexInput : public ResourceManager::Resource<VertexInputId>
{
	static auto RegisterResourceHandler(ResourceManager::ResourceMan& rm_) -> void;
	static constexpr uint16_t MajorVersion = 1;
	static constexpr uint16_t MinorVersion = 0;

	using Input = struct
	{
		VertexInputLocation location;
		VertexInputType type;
		uint8_t padd[2];
	};

	static auto Create(
			std::shared_ptr<ResourceManager::ResourceMan> rm_,
			ResourceManager::ResourceNameView const& name_,
			std::vector<Input> const& inputs_) -> VertexInputHandle;

	Input const* getInputs() const { return (Input const*) (this + 1); }

	auto getStride() const -> uint32_t;

	auto getElementSize(uint8_t const index_) const -> uint32_t;
	auto getElementOffset(uint8_t const index_) const -> uint32_t;

	uint8_t numVertexInputs;
	uint8_t padd[7];

};

}

#endif //WYRD_RENDER_VERTEXINPUT_H
