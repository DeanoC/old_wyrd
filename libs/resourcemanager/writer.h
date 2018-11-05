#pragma once
#ifndef WYRD_RESOURCEMANANAGER_WRITER_H
#define WYRD_RESOURCEMANANAGER_WRITER_H

#include "core/core.h"
#include "binny/writehelper.h"
#include "resourcemanager/resource.h"
#include "resourcemanager/resourceman.h"

namespace ResourceManager {

class Writer : public Binny::WriteHelper
{
public:

	using Binny::WriteHelper::write;
	auto write(ResourceHandleBase const& base_) -> void
	{
		auto name = ResourceMan::GetNameFromHandleBase(base_);
		if(name.isValid() || name.isNull())
		{
			add_string("null$null");
		} else
		{
			add_string(name.getResourceName());
		}

	}
};

}
#endif //WYRD_RESOURCEMANANAGER_WRITER_H
