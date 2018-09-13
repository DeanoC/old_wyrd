//
// Created by Computer on 26/07/2018.
//

#pragma once
#ifndef MESHMOD_VARIELEMENTS_H
#define MESHMOD_VARIELEMENTS_H

#include "core/core.h"
#include <string>
#include <vector>

namespace MeshMod {

enum DerivedType
{
	NotDerived = 0,
	DerivedFromTopology,
	DerivedFromPositions,
	DerivedFromAttributes,
};

/**
Short description.
Detailed description
*/
template<typename T>
class Elements
{
public:
	virtual ~Elements()
	{};

	// name of this type of element ("position", "UV2D", "Colour", etc)
	std::string const name;
	// subname. changeable for multiple sets of m_name's ("worldspace", "Tex0",etc)
	std::string subName;

	// this only works if the dest is the same as the source else does nothing!
	virtual void unsafeCopyElementTo(Elements<T>& dest, size_t srcIndex, size_t destIndex) = 0;

	virtual void cloneElement(size_t const index) = 0;

	virtual void resize(size_t const size) = 0;

	virtual size_t size() const = 0;

	virtual std::shared_ptr<Elements<T>> clone() const = 0;

	virtual DerivedType derived() const = 0;

protected:
	Elements(std::string const& _name)
			: name(_name)
	{};

	Elements &operator=(Elements const&);

};

template<typename DT, typename CT, bool Interpol, DerivedType Derived, int reserveAmnt = 1024>
class BaseElements : public Elements<CT>
{
public:
	typedef DT DataType;
	typedef CT ContainerType;
	typedef typename std::vector<DataType>::iterator iterator;
	typedef typename std::vector<DataType>::const_iterator const_iterator;
	typedef typename BaseElements<DT, CT, Interpol, Derived, reserveAmnt> BaseElementType;

	std::vector<DataType> elements;

	BaseElements() : Elements<ContainerType>(DataType::getName())
	{
		elements.reserve(reserveAmnt);
	}

	static std::string const getName()
	{
		return DataType::getName();
	}

	DataType const &get(size_t const i) const
	{
		return elements[i];
	};

	DataType &get(size_t const i)
	{
		return elements[i];
	};


	DataType const &operator[](size_t const i) const
	{
		return get(i);
	};

	DataType &operator[](size_t const i) { return get(i); };

	iterator begin() { return elements.begin(); }
	iterator end() { return elements.end(); }
	const_iterator begin() const { return elements.begin(); }
	const_iterator end() const { return elements.end(); }

	const_iterator cbegin() const { return elements.cbegin(); }
	const_iterator cend() const { return elements.cend(); }

	void erase(const_iterator where_) { elements.erase(where_); }
	void erase(const_iterator first_, const_iterator last_) { elements.erase(first_, last_); }

	void push_back(DataType const &data)
	{
		elements.push_back(data);
	};

	template<typename IndexType>
	IndexType distance(const_iterator it) const
	{
		return (IndexType) std::distance(elements.cbegin(), it);
	}

	template<typename IndexType>
	IndexType distance(DataType const &data) const
	{
		DataType const* beginPtr = &(*elements.cbegin());
		IndexType index = (PolygonIndex)(&data - beginPtr);
		return index;
	}


	template<typename INTER_TYPE>
	DataType interpolate(size_t const i0, size_t const i1, INTER_TYPE t)
	{
		return elements[i0].interpolate(elements[i1], t);
	}

	template<typename INTER_TYPE>
	DataType interpolate(size_t const i0, size_t const i1, size_t const i2, INTER_TYPE u, INTER_TYPE v)
	{
		return elements[i0].interpolate(elements[i1], elements[i2], u, v);
	}

	//--------
	// Elements interface implementation
	//--------

	virtual void cloneElement(const size_t index) override
	{
		elements.push_back(elements[index]);
	};

	virtual void unsafeCopyElementTo(Elements<CT>& dest, size_t srcIndex, size_t destIndex) override
	{
		assert(srcIndex< elements.size());

		BaseElementType* ptr = dynamic_cast<BaseElementType*>(&dest);
		if (ptr)
		{
			assert(destIndex < ptr->elements.size());
			ptr->elements[destIndex] = elements[srcIndex];
		}
	}
	virtual void resize(const size_t size) override
	{
		elements.resize(size);
	};

	virtual size_t size() const override
	{
		return elements.size();
	}

	virtual std::shared_ptr<Elements<CT>> clone() const override
	{
		return std::make_shared<BaseElementType>(*this);
	}

	virtual DerivedType derived() const override
	{
		return Derived;
	}

};

}

#endif //MESHMOD_VARIELEMENTS_H
