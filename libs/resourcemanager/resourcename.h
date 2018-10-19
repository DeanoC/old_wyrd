#pragma once
#ifndef WYRD_RESOURCEMANANAGER_RESOURCENAME_H
#define WYRD_RESOURCEMANANAGER_RESOURCENAME_H

#include "core/core.h"
#include "tbb/concurrent_hash_map.h"
#include "cityhash/city.h"
namespace ResourceManager
{

// resource name are made of 3 parts (the last optional)
// 1) the storage
// 2) the name / filename of a 'bundle'
// 3) the sub object
// the $ is used to delimit the 3 parts and cannot be used by part names
// the |$|$ sequence is reserved and used inside bundles to refer to the bundle

// the type should be short and is used to select the storage system to retrieve
// the resource, e.g. disk or mem. null storage is special always available nullptr
// for the null resource, the name is ignored but for stylistic reason null$null is
// the prefered null reference indiacator

// the name used by the storage system to locate the main bundle object
// a filename or description are examples

// the third is optional and allows access to sub-objects
// by default if not supplied the first resource in the bundle of the correct type
// is returned, if the third part is supplied then the name will be matched to the
// bundles directory name.

// inside a bundle the first and second parts set to |$|$ refer to the bundle (and
// storage) itself.

// examples
// disk$/asset/texture/bob
// mem$BobTheHero
// disk$/asset/texture/bob$leg1
// |$|$leg1 - legal inside a bundle to refer to this bundle and storage system
// null$null - a nullptr/invalid resource handle.

struct ResourceNameView
{
	friend struct ResourceName;

	ResourceNameView(std::string_view in_) : resourceName(in_)
	{
		dollar0pos = resourceName.find_first_of('$');
		dollar1pos = resourceName.find_first_of('$', dollar0pos+1);
	}

	bool isValid() const
	{
		return 	dollar0pos != resourceName.npos &&
				  dollar0pos+1 != dollar1pos &&
				  dollar0pos+1 != resourceName.size();
	}

	auto isCurrentLink() const -> bool
	{
		return isValid() &&
			resourceName[0] == '|' &&
			resourceName[2] == '|' &&
			dollar1pos != resourceName.npos &&
			dollar1pos+1 != resourceName.size();
	}

	auto isNull() const -> bool {
		return isValid() && getStorage() == "null";
	}

	std::string_view getStorage() const
	{
		std::string_view view( resourceName );
		view.remove_suffix(resourceName.size() - dollar0pos);
		return view;
	}

	std::string_view getName() const
	{
		std::string_view view( resourceName );
		view.remove_prefix(dollar0pos+1);
		if(dollar1pos != resourceName.npos)
		{
			view.remove_suffix(resourceName.size() - dollar1pos);
		}
		return view;
	}

	std::string_view getSubObject() const
	{

		if(dollar1pos != resourceName.npos)
		{
			std::string_view view( resourceName );
			view.remove_prefix(dollar1pos+1);
			return view;
		} else
		{
			return {};
		}
	}

	std::string_view getNameAndSubObject() const
	{
		std::string_view view( resourceName );
		view.remove_prefix(dollar0pos+1);
		return view;
	}

	std::string_view getResourceName() const
	{
		return resourceName;
	};
	bool operator==(ResourceNameView const& rhs) const
	{
		return resourceName == rhs.resourceName;
	}

	bool operator!=(ResourceNameView const& rhs) const
	{
		return resourceName != rhs.resourceName;
	}

	bool operator<(ResourceNameView const& rhs) const
	{
		return resourceName < rhs.resourceName;
	}

protected:
	ResourceNameView(){} // used by one of the resource name constuctors

	std::string::size_type dollar0pos;
	std::string::size_type dollar1pos;
	std::string_view resourceName;
};

struct ResourceName
{
	ResourceName() {}

	explicit ResourceName(ResourceNameView const view_) : ResourceName(view_.resourceName) {}

	explicit ResourceName(std::string_view storage_, std::string_view name_, std::string_view subObject_)
	{
		resourceName.reserve(storage_.size() + name_.size() + subObject_.size() + 2);
		resourceName = storage_;
		resourceName += "$";
		resourceName += name_;
		if(!subObject_.empty())
		{
			resourceName += "$";
			resourceName += subObject_;
		}
	}

	ResourceName(std::string_view in) : resourceName(std::string(in))
	{
		if(getView().dollar0pos == resourceName.npos)
		{
			// default to disk
			resourceName = "disk$" + resourceName;
		}
		assert(getView().dollar0pos != resourceName.npos);
	}

	ResourceNameView getView() const { return ResourceNameView(resourceName); }

	auto isValid() -> bool const { return getView().isValid(); }

	auto isCurrentLink() -> bool const { return getView().isCurrentLink(); }

	auto isNull() -> bool const { return getView().isNull(); }

	std::string_view getStorage() const { return getView().getStorage(); }

	std::string_view getName() const { return getView().getName(); }

	std::string_view getSubObject() const { return getView().getSubObject(); }

	std::string_view getNameAndSubObject() const { return getView().getNameAndSubObject(); }

	std::string_view getResourceName() const { return resourceName; }

	bool operator==(ResourceName const& rhs) const
	{
		return resourceName == rhs.resourceName;
	}

	bool operator!=(ResourceName const& rhs) const
	{
		return resourceName != rhs.resourceName;
	}

	bool operator<(ResourceName const& rhs) const
	{
		return resourceName < rhs.resourceName;
	}

protected:
	std::string resourceName;
};


} // end namespace

namespace std
{
template<> struct hash<ResourceManager::ResourceName>
{
	std::size_t operator()( ResourceManager::ResourceName const & name_) const
	{
		return hash<std::string_view>()(name_.getResourceName());
	}
};
template<> struct hash<ResourceManager::ResourceNameView>
{
	std::size_t operator()( ResourceManager::ResourceNameView const & name_) const
	{
		return hash<std::string_view>()(name_.getResourceName());
	}
};

}

namespace tbb
{
template<> class tbb_hash<ResourceManager::ResourceName>
{
public:
	size_t operator()( ResourceManager::ResourceName const & name_) const
	{
		return CityHash::Hash64(name_.getResourceName().data(), name_.getResourceName().length());
	}
};

template<> class tbb_hash<ResourceManager::ResourceNameView>
{
public:
	size_t operator()( ResourceManager::ResourceNameView const & name_) const
	{
		return CityHash::Hash64(name_.getResourceName().data(), name_.getResourceName().length());
	}
};

}

#endif
