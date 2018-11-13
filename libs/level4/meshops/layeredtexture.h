/// \brief	Declares the layeredtexture class.
/// \remark	Copyright (c) 2018 Dean Calver. Public Domain.
/// \remark	mailto://deano@cloudpixies.com

#pragma once
#ifndef MESHOPS_LAYEREDTEXTURE_H
#define MESHOPS_LAYEREDTEXTURE_H

#include <map>
#include <vector>
#include <memory>
#include <string_view>
namespace MeshOps {

/// \class	TextureLayerInterface
/// \brief	A named layer, size is defined by its owning LayeredTexture
struct ITextureLayer
{
	virtual ~ITextureLayer() = default;
	virtual std::string_view getName() const = 0;

	virtual std::type_info const && getComponentType() const  = 0;
	virtual size_t getComponentSize() const = 0;
	virtual unsigned int getComponentCount() const = 0;
	virtual unsigned int getWidth() const = 0;
	virtual unsigned int getHeight() const = 0;

	virtual uint8_t const * getByteData() const = 0;
	virtual uint8_t * getByteData() = 0;

	template<typename Type> Type* getData() { 
		assert(typeid(Type) == getComponentType());
		return (Type*) getByteData();
	}
	template<typename Type> Type const* getData() const 
	{ 
		assert(typeid(Type) == getComponentType());
		return (Type const*) getByteData();
	}

	template<typename Type>
	Type const getAt(unsigned int x, unsigned int y, int component = 0) const
	{
		assert(typeid(Type) == getComponentType());
		size_t const index = (((y * getWidth()) + x) * getComponentCount()) + component;
		return *(Type*)(getByteData() + index * getComponentSize());
	}

	template<typename Type>
	void setAt(unsigned int x, unsigned int y,  int component, Type const& item)
	{
		assert(typeid(Type) == getComponentType());
		size_t const index = (((y * getWidth()) + x) * getComponentCount()) + component;
		std::memcpy(getByteData() + index * getComponentSize(), &item, getComponentSize());
	}
};

/// \class	TextureLayer
/// \brief	A named layer, size is defined by its owning LayeredTexture
template<typename T>
class TextureLayer : public ITextureLayer {
public:
	std::string_view getName() const final { return name; }

	std::type_info const &&getComponentType() const  final { return std::move(typeid(T)); };
	size_t getComponentSize() const final { return sizeof(T); };
	unsigned int getComponentCount() const final { return componentCount; }
	unsigned int getWidth() const final { return width; }
	unsigned int getHeight() const final { return height; }

	T const * getData() const { return dataStore.data(); }
	T* getData() { return dataStore.data(); }

	uint8_t const * getByteData() const final { return (uint8_t const*) dataStore.data(); }
	uint8_t * getByteData() final { return (uint8_t*) dataStore.data(); }

	std::vector<T>& getDataStore() { return dataStore; }

	TextureLayer( unsigned int width_, unsigned int height_, std::string const& name_, unsigned int componentCount_ ) :
			name( name_ ), width(width_), height(height_), componentCount( componentCount_ ),
			dataStore( width_ * height_ * componentCount_ ){}

	TextureLayer<T>&& operator=( TextureLayer<T> const& rhs)
	{
		TextureLayer<T> newLayer;
		newLayer.name = rhs.name;
		newLayer.width = rhs.width;
		newLayer.height = rhs.height;
		newLayer.dataStore = rhs.dataStore;
		newLayer.componentCount = rhs.componentCount;
		return std::move(newLayer);
	}

private:
	TextureLayer() {};

	std::string const name;
	unsigned int const width;
	unsigned int const height;

	std::vector<T> dataStore;
	unsigned int componentCount;		//!< Number of components per pixel
};


/// \class	LayeredTexture
/// \brief	layered texture. Holds layers of the same size image (components per pixel and dimension)
/// 		Each layer is named, allowing complex multi-layer textures to be passed around
class LayeredTexture {
public:
	LayeredTexture( unsigned int width_, unsigned int height_ ) :
			width( width_ ), height( height_ ) {};

	/// \brief	Gets the width.
	/// \return	The width.
	unsigned int getWidth() const { return width; }

	/// \brief	Gets the height.
	/// \return	The height.
	unsigned int getHeight() const { return height; }

	/// \brief	Gets the layer count.
	/// \todo	Fill in detailed method description.
	/// \return	The layer count.
	unsigned int getLayerCount() const { return (unsigned int) layers.size(); }

	/// \brief	Gets a layer.
	/// \param	name index.
	/// \return	The layer at specified index.
	ITextureLayer const& getLayer( size_t index) const
	{
		assert(index < layers.size());
		return *(layers[index].get());
	}

	/// \brief	Gets a layer.
	/// \param	name index.
	/// \return	The layer at specified index.
	ITextureLayer& getLayer( size_t index)
	{
		assert(index < layers.size());
		return *(layers[index].get());
	}

	/// \brief	Gets a layer.
	/// \param	name	The name.
	/// \return	The layer.
	ITextureLayer const& getLayer( std::string const& name ) const
	{
		assert(layersNameMap.find(name) != layersNameMap.end());
		size_t index = layersNameMap.find(name)->second;
		return *(layers[index].get());
	}

	/// \brief	Gets a layer.
	/// \param	name	The name.
	/// \return	The layer.
	ITextureLayer& getLayer( std::string const& name )
	{
		assert(layersNameMap.find(name) != layersNameMap.end());
		size_t index = layersNameMap.find(name)->second;
		return *(layers[index].get());
	}

	/// \brief	Adds a layer.
	/// \param	name		   	The name.
	/// \param	_componentCount	Number of per pixel components.
	/// \return	null if it fails, else.
	template<typename Type>
	ITextureLayer& addLayer( std::string const& name, unsigned int _componentCount )
	{
		assert(layersNameMap.find(name) == layersNameMap.end());

		size_t index = layers.size()-1;
		auto* newLayer = new TextureLayer<Type>(width, height, name, _componentCount);
		layers.emplace_back(newLayer);
		layersNameMap[name] = index;
		return *newLayer;
	}

private:
	unsigned int width;					//!< The width
	unsigned int height;				//!< The height
	using LayerList = std::vector<std::unique_ptr<ITextureLayer>>;
	using LayerNameMap = std::map<std::string, size_t>;

	LayerList layers; //!< The layers
	LayerNameMap layersNameMap;
};

using FloatTextureLayer = TextureLayer<float>;

} //end namespace


#endif //MESHOPS_LAYEREDTEXTURE_H
