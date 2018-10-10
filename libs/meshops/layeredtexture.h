/// \brief	Declares the layeredtexture class.
/// \remark	Copyright (c) 2018 Dean Calver. Public Domain.
/// \remark	mailto://deano@cloudpixies.com

#pragma once
#ifndef MESHOPS_LAYEREDTEXTURE_H
#define MESHOPS_LAYEREDTEXTURE_H

#include <map>
#include <vector>
#include <memory>
namespace MeshOps {

/// \class	TextureLayer
/// \brief	A named layer, size is defined by its owning LayeredTexture
template<typename T>
class TextureLayer {
public:
	std::string const & getName() const { return name; }
	T const * getData() const { return dataStore.data(); }
	T* getData() { return dataStore.data(); }

	std::vector<T>& getDataStore() { return dataStore; }
	/// \fn	unsigned int FloatLayer::getComponentCount() const
	/// \brief	Gets the component per pixel count.
	/// \return	The component count.
	unsigned int getComponentCount() const { return componentCount; }

	TextureLayer( unsigned int width_, unsigned int height_, std::string const& name_, unsigned int componentCount_ ) :
			name( name_ ), width(width_), height(height_), componentCount( componentCount_ ),
			dataStore( width * height_ * componentCount_ ){}

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

	T const& getAt(unsigned int x, unsigned int y, int component = 0) const { return dataStore[(((y * width) + x) * componentCount) + component]; }
	void setAt(unsigned int x, unsigned int y,  int component, T const& item){ dataStore[(((y * width) + x) * componentCount) + component] = item; }

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
template<typename T>
class LayeredTexture {
public:
	LayeredTexture( unsigned int _width, unsigned int _height ) : width( _width ), height( _height ) {};

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
	TextureLayer<T> const& getLayer( size_t index) const
	{
		assert(index < layers.size());
		return *(layers[index].get());
	}

	/// \brief	Gets a layer.
	/// \param	name index.
	/// \return	The layer at specified index.
	TextureLayer<T>& getLayer( size_t index)
	{
		assert(index < layers.size());
		return *(layers[index].get());
	}

	/// \brief	Gets a layer.
	/// \param	name	The name.
	/// \return	The layer.
	TextureLayer<T> const& getLayer( std::string const& name ) const
	{
		assert(layersNameMap.find(name) != layersNameMap.end());
		size_t index = layersNameMap.find(name)->second;
		return *(layers[index].get());
	}

	/// \brief	Gets a layer.
	/// \param	name	The name.
	/// \return	The layer.
	TextureLayer<T>& getLayer( std::string const& name )
	{
		assert(layersNameMap.find(name) != layersNameMap.end());
		size_t index = layersNameMap.find(name)->second;
		return *(layers[index].get());
	}

	/// \brief	Adds a layer.
	/// \param	name		   	The name.
	/// \param	_componentCount	Number of per pixel components.
	/// \return	null if it fails, else.
	TextureLayer<T>& addLayer( std::string const& name, unsigned int _componentCount )
	{
		assert(layersNameMap.find(name) == layersNameMap.end());

		size_t index = layers.size()-1;
		layers.emplace_back(std::make_unique<TextureLayer<T>>( width, height, name, _componentCount));
		layersNameMap[name] = index;
		return *(layers.back().get());
	}

	LayeredTexture<T>&& operator=( LayeredTexture<T> const& rhs)
	{
		TextureLayer<T> newTex(rhs.width, rhs.height);
		newTex.layers = rhs.layers;
		newTex.layersNameMap = rhs.layersNameMap;
		return std::move(newTex);
	}
private:
	unsigned int width;					//!< The width
	unsigned int height;				//!< The height
	using LayerList = std::vector<std::unique_ptr<TextureLayer<T>>>;
	using LayerNameMap = std::map<std::string, size_t>;

	LayerList layers; //!< The layers
	LayerNameMap layersNameMap;
};

using FloatTextureLayer = TextureLayer<float>;
using LayeredFloatTexture = LayeredTexture<float>;

} //end namespace


#endif //MESHOPS_LAYEREDTEXTURE_H
