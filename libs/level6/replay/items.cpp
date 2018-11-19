#include "core/core.h"
#include "core/quick_hash.h"
#include "replay/items.h"
#include <functional>
#include <cctype>

namespace Replay::Items
{
LogItem::LogItem(nlohmann::json const& j_)
{
	auto jtext = j_.find("text");
	if(jtext->is_null()) return;

	if(jtext->is_string())
	{
		text = jtext->get<std::string>();
	}
	auto jgroup = j_.find("group");
	if(jgroup->is_string())
	{
		group = jgroup->get<std::string>();
	}
	auto jlevel = j_.find("level");
	if(jlevel->is_string())
	{
		using namespace Core;
		auto levelString = jlevel->get<std::string>();
		std::transform(levelString.cbegin(),
				levelString.cend(), levelString.begin(), std::tolower);
		switch(Core::QuickHash(levelString))
		{
			case "info"_hash: level = Level::Info; break;
			case "warning"_hash: level = Level::Warning; break;

			default:
			case "error"_hash: level = Level::Error; break;
		};
	}

}

}

