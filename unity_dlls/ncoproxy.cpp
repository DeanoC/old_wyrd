#include "core/core.h"
#include "core/quick_hash.h"
#include "core/filesystem.h"
#include <functional>
#include <array>

using DLLInterfaceFunc = void*(*)();

// we don't want any lasting allocations, so use static blocks of memory instead of allocs
constexpr int MaxModules = 16;

namespace {
	std::array<uint32_t, MaxModules> s_moduleNameHashs;
#if PLATFORM == WINDOWS
	std::array<HMODULE, MaxModules> s_modules;
#endif
	uint32_t s_maxModuleIndex = 0;
} // anon namespace

#if PLATFORM == WINDOWS
// we delibrately don't couple the interfaces so that this doesn't get rebuilt except when a
// new library is added
auto getInterface(char const * name_) -> void*
{
	DLLInterfaceFunc func = nullptr;
	auto hash = Core::QuickHash(name_, strlen(name_));
	for (size_t i = 0; i < s_maxModuleIndex; i++)
	{
		if (hash == s_moduleNameHashs[i])
		{
			func = (DLLInterfaceFunc)GetProcAddress(s_modules[i], "GetInterface");
 			break;
		}
	}

	if(func != nullptr)
	{
		return func();
	}

	return nullptr;
}
#endif

using InterfaceFunc = void*(*)(char const*);

EXPORT auto Finish() -> void
{
#if PLATFORM == WINDOWS
	for (size_t i = 0; i < s_maxModuleIndex; i++)
	{
		FreeLibrary(s_modules[i]);
	}
#endif
	s_maxModuleIndex = 0;

}

EXPORT auto Init() -> InterfaceFunc
{
	// TODO mac and linux
#if PLATFORM == WINDOWS
	namespace fs = std::filesystem;
	using namespace std::string_literals;

	// clean up and previous runs where Finish was missed (stopping Unity for example)
	Finish();

	for (auto& p : fs::recursive_directory_iterator("Assets/Plugins/"))
	{
		if (fs::is_regular_file(p) && p.path().extension().string() == ".nco"s)
		{
			// 'allocate' and copy filename
			std::string const filename = p.path().filename().string();
			std::string_view v = filename;
			v.remove_suffix(4);
			s_moduleNameHashs[s_maxModuleIndex] = Core::QuickHash(v);
			s_modules[s_maxModuleIndex] = LoadLibrary(p.path().string().c_str());
			if (s_modules[s_maxModuleIndex] != 0)
			{
				s_maxModuleIndex++;
			}
		}
	}
#endif
	return &getInterface;
}

