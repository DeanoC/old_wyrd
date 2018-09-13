#pragma once
/** \file VariContainer.h
   Used by the mesh system for holding variable amounts of vectors (vertex, faces).
   (c) 2002 Dean Calver
 */

#if !defined( MESH_MOD_VARICONTAINER_H_ )
#define MESH_MOD_VARICONTAINER_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "core/core.h"
#include <string>
#include <vector>
#include "varielements.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace MeshMod {

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------


template<typename T>
class VariContainer
{
public:
	typedef T ContainerType;
	typedef std::shared_ptr<T> Ptr;
	typedef std::shared_ptr<const T> ConstPtr;

	~VariContainer() { clear(); }

	void clear();

	bool isValid(size_t elementIndex) const
	{
		if(elementIndex == MM_INVALID_INDEX) return false;
		return !notValidFlags.at(elementIndex);
	}
	void setValid(size_t elementIndex, bool valid)
	{
		notValidFlags.at(elementIndex) = !valid;
	}
	void resetValidFlags() {
		size_t osize = size();
		notValidFlags.clear(); notValidFlags.resize(osize);
	}
	size_t resizeForNewElement();

	void resize(size_t const size);

	size_t size() const;

	void cloneTo(VariContainer<T> &nvc) const;

	//! add the element with default subname
	Ptr addElements(typename Ptr ele, std::string const &subName = std::string());

	//! remove the elements
	void removeElements(typename Ptr ele);

	//! gets the first element named name (ther may be others with different subnames)
	Ptr getElements(std::string const &name);

	//! gets the first element named name (ther may be others with different subnames) (const)
	ConstPtr getElements(std::string const &name) const;

	//! gets all elements of the input name
	void getAllElementsNamed(std::string const &name, std::vector<Ptr> &out);

	//! gets all elements of the input name (const)
	void getAllElementsNamed(std::string const &name, std::vector<ConstPtr> &out) const;

	// gets the element of name and sub name
	Ptr getElementsNameAndSubName(std::string const &name, std::string const &subName);

	// gets the element of name and sub name (const_
	ConstPtr getElementsNameAndSubName(std::string const &name, std::string const &subName) const;

	// how many elements do we have.
	size_t getSizeOfElementContainer() const
	{
		return elements.size();
	}
	Ptr getElementContainer(size_t index) { return elements[index]; }

	template<typename Type>
	std::shared_ptr<Type> addElements(std::string const &subName = std::string())
	{
		std::shared_ptr<Type> ele = std::static_pointer_cast<Type>(addElements(std::make_shared<Type>()));
		assert(ele);
		return ele;
	}

	template<typename Type>
	std::shared_ptr<Type> getOrAddElements(std::string const &subName = std::string())
	{
		std::shared_ptr<Type> ele;
		if(!subName.empty())
		{
			ele = std::static_pointer_cast<Type>(getElementsNameAndSubName(Type::getName(), subName));
			if(!ele)
			{
				ele = std::static_pointer_cast<Type>(addElements(std::make_shared<Type>(), subName));
			}
		} else
		{
			ele = std::static_pointer_cast<Type>(getElements(Type::getName()));
			if(!ele)
			{
				ele = std::static_pointer_cast<Type>(addElements(std::make_shared<Type>()));
			}
		}

		assert(ele);
		return ele;
	}

	template<typename Type>
	void pushBack(std::string const &subName = std::string(),
				  typename Type::DataType const &data = typename Type::DataType())
	{
		// if we adding a new type we don't wanna always resize the buffer after the add
		// cos adding a new type resizes it to the size of the array. This encapsulates
		// the slight complicated and quite frankly odd logic.. it basically does
		// what I expect by pushBack
		if(getElementsNameAndSubName(Type::getName(), subName) == 0)
		{
			std::shared_ptr<Type> pType = getOrAddElements<Type>(subName);
			if(pType->size() == 0)
			{
				pType->push_back(data);
			} else
			{
				pType->getElement(0) = data;
			}
		} else
		{
			Type *pType = getElements<Type>(subName);
			pType->push_back(data);
		}
	}

	template<typename Type>
	std::shared_ptr<Type> getElements()
	{
		return std::static_pointer_cast<Type>(getElements(Type::DataType::getName()));
	}

	template<typename Type>
	std::shared_ptr<Type const> getElements() const
	{
		return std::static_pointer_cast<Type const>(getElements(Type::DataType::getName()));
	}

	template<typename Type>
	std::shared_ptr<Type> getElements(std::string const &subName)
	{
		return std::static_pointer_cast<Type>(getElementsNameAndSubName(Type::DataType::getName(), subName));
	}

	template<typename Type>
	std::shared_ptr<const Type>getElements(std::string const &subName) const
	{
		return std::static_pointer_cast<Type const>(getElementsNameAndSubName(Type::DataType::getName(), subName));
	}

	//! gets all elements of the types name
	template<typename Type>
	void getAllElements(std::vector<std::shared_ptr<Type>> &out)
	{
		typename std::vector<Ptr>::iterator feIt = elements.begin();
		while(feIt != elements.end())
		{
			if((*feIt)->name == Type::DataType::getName())
			{
				out.push_back(std::static_pointer_cast<Type>(*feIt));
			}
			++feIt;
		}
	}

	//! gets all elements of the types name (const)
	template<typename Type>
	void getAllElements(std::vector<Type const *> &out) const
	{
		typename std::vector<ConstPtr>::const_iterator feIt = elements.begin();
		while(feIt != elements.end())
		{
			if((*feIt)->name == Type::DataType::getName())
			{
				out.push_back(std::static_pointer_cast<std::shared_ptr<Type>>(*feIt));
			}
			++feIt;
		}
	}

	size_t cloneElement(size_t const elementToCopy);

	void removeDerived(DerivedType change)
	{
		assert(change != DerivedType::NotDerived);

		std::vector<Ptr> derivedElements;
		// wipe derived data
		for(size_t i = 0; i < getSizeOfElementContainer(); ++i)
		{
			Ptr elementContainer = getElementContainer(i);
			if(elementContainer->derived() >= change)
			{
				derivedElements.push_back(elementContainer);
			}
		}

		for(auto toRemove : derivedElements)
		{
			auto& it = std::find(elements.begin(), elements.end(), toRemove);
			if(it != elements.end())
			{
				elements.erase(it);
			}
		}
	}

private:
	// elements
	std::vector<Ptr> elements;

	std::vector<bool> notValidFlags; // yes I know vector bool aren't the greatest
};

template<typename T>
inline void VariContainer<T>::clear()
{
	elements.clear();
	notValidFlags.clear();
}

template<typename T>
inline size_t VariContainer<T>::size() const
{
	return notValidFlags.size();
}

template<typename T>
inline size_t VariContainer<T>::resizeForNewElement()
{
	size_t preSize = notValidFlags.size();
	resize(preSize + 1);
	return preSize;
}

template<typename T>
inline void VariContainer<T>::resize(size_t const size)
{
	notValidFlags.resize(size);

	typename std::vector<Ptr>::iterator veIt = elements.begin();
	while(veIt != elements.end())
	{
		(*veIt)->resize(size);
		++veIt;
	}
}

template<typename T>
inline typename VariContainer<T>::Ptr VariContainer<T>::addElements(typename Ptr ele, std::string const &subName)
{
	elements.push_back(ele);
	ele->resize(notValidFlags.size());
	ele->subName = subName;
	return ele;
}

template<typename T>
inline void VariContainer<T>::removeElements(typename Ptr ele)
{
	auto& it = std::find(elements.begin(), elements.end(), ele);
	if (it != elements.end())
	{
		elements.erase(it);
	}
}


template<typename T>
inline typename VariContainer<T>::Ptr VariContainer<T>::getElements(std::string const &name)
{
	typename std::vector<Ptr>::iterator feIt = elements.begin();

	while(feIt != elements.end())
	{
		if((*feIt)->name == name)
		{
			return (*feIt);
		}
		++feIt;
	}

	return nullptr;
}

template<typename T>
inline typename VariContainer<T>::ConstPtr VariContainer<T>::getElements(std::string const &name) const
{
	typename std::vector<Ptr>::const_iterator feIt = elements.begin();

	while(feIt != elements.end())
	{
		if((*feIt)->name == name)
		{
			return (*feIt);
		}
		++feIt;
	}

	return nullptr;
}


template<typename T>
inline void VariContainer<T>::getAllElementsNamed(std::string const &name, std::vector<Ptr> &out)
{
	typename std::vector<Ptr>::iterator feIt = elements.begin();

	while(feIt != elements.end())
	{
		if((*feIt)->name == name)
		{
			out.push_back((*feIt));
		}
		++feIt;
	}
}

template<typename T>
inline void VariContainer<T>::getAllElementsNamed(std::string const &name,
												  std::vector<ConstPtr> &out) const
{
	typename std::vector<T *>::const_iterator feIt = elements.begin();

	while(feIt != elements.end())
	{
		if((*feIt)->name == name)
		{
			out.push_back((*feIt));
		}
		++feIt;
	}
}

template<typename T>
inline typename VariContainer<T>::Ptr VariContainer<T>::getElementsNameAndSubName(std::string const &name,
													  std::string const &subname)
{
	typename std::vector<Ptr>::iterator feIt = elements.begin();

	while(feIt != elements.end())
	{
		if(((*feIt)->name == name) &&
		   ((*feIt)->subName == subname))
		{
			return (*feIt);
		}
		++feIt;
	}

	return nullptr;
}

template<typename T>
inline typename VariContainer<T>::ConstPtr VariContainer<T>::getElementsNameAndSubName(std::string const &name,
															std::string const &subname) const
{
	typename std::vector<Ptr>::const_iterator feIt = elements.begin();

	while(feIt != elements.end())
	{
		if(((*feIt)->name == name) &&
		   ((*feIt)->subName == subname))
		{
			return (*feIt);
		}
		++feIt;
	}

	return ConstPtr();
}

template<typename T>
inline size_t VariContainer<T>::cloneElement(size_t const elementToCopy)
{
	typename std::vector<Ptr>::iterator feIt = elements.begin();
	notValidFlags.push_back(false);

	while(feIt != elements.end())
	{
		(*feIt)->cloneElement(elementToCopy);
		++feIt;
	}

	return size() - 1;
}

template<typename T>
inline void VariContainer<T>::cloneTo(VariContainer<T> &nvc) const
{
	typename std::vector<Ptr>::const_iterator feIt = elements.cbegin();
	nvc.clear();
	nvc.notValidFlags = notValidFlags;

	while(feIt != elements.cend())
	{
		nvc.elements.push_back((*feIt)->clone());
		++feIt;
	}
}

} // end namespace
#endif
