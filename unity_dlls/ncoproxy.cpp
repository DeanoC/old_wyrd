#include "core/core.h"
#include "core/quick_hash.h"

#if PLATFORM_OS == OSX
#include "cppfs/cppfs.h"
#include "cppfs/fs.h"
#include "cppfs/FileHandle.h"
#include "cppfs/FileIterator.h"
#include "cppfs/FilePath.h"
#else
#include "core/filesystem.h"
#endif
#include <functional>
#include <array>

#if PLATFORM != WINDOWS
#include <dlfcn.h>
#endif

using DLLInterfaceFunc = void*(*)();

// we don't want any lasting allocations, so use static blocks of memory instead of allocs
constexpr int MaxModules = 16;

namespace {
	std::array<uint32_t, MaxModules> s_moduleNameHashs;
#if PLATFORM == WINDOWS
	std::array<HMODULE, MaxModules> s_modules;
#else
	std::array<void*, MaxModules> s_modules;
#endif
	uint32_t s_maxModuleIndex = 0;
} // anon namespace

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
#if PLATFORM == WINDOWS
			func = (DLLInterfaceFunc)GetProcAddress(s_modules[i], "GetInterface");
#else
			func = (DLLInterfaceFunc)dlsym(s_modules[i], "GetInterface");
#endif
 			break;
		}
	}

	if(func != nullptr)
	{
		return func();
	}

	return nullptr;
}

using InterfaceFunc = void*(*)(char const*);

EXPORT auto Finish() -> void
{
	for (size_t i = 0; i < s_maxModuleIndex; i++)
	{
#if PLATFORM == WINDOWS
		FreeLibrary(s_modules[i]);
#else
		dlclose(s_modules[i]);
#endif
	}
	s_maxModuleIndex = 0;

}

auto LoadModule(std::string const& filename_, std::string const& path_) -> void
{
	// 'allocate' and copy filename
	std::string_view v = filename_;
	v.remove_suffix(4);
	s_moduleNameHashs[s_maxModuleIndex] = Core::QuickHash(v);
#if PLATFORM == WINDOWS
	s_modules[s_maxModuleIndex] = LoadLibrary(path_.c_str());
#else
	s_modules[s_maxModuleIndex] = dlopen(path_.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
	if (s_modules[s_maxModuleIndex] != 0)
	{
		s_maxModuleIndex++;
	}
}
EXPORT auto Init() -> InterfaceFunc
{
	// TODO mac and linux
	using namespace std::string_literals;

	// clean up and previous runs where Finish was missed (stopping Unity for example)
	Finish();

#if PLATFORM_OS == OSX
	using namespace cppfs;

	FileHandle dir = fs::open("Assets/Plugins/");
	if (dir.isDirectory())
	{
		for (FileIterator it = dir.begin(); it != dir.end(); ++it)
		{
			std::string pathstr = *it;
			FilePath path(pathstr);
			FileHandle f = dir.open(*it);
			if(f.isFile() && path.extension() == ".nco"s)
			{
				LoadModule(path.fileName(), path.fullPath());
			}
		}
	}
#else
	namespace fs = std::filesystem;
	for (auto& p : fs::recursive_directory_iterator("Assets/Plugins/"))
	{
		if (fs::is_regular_file(p) && p.path().extension().string() == ".nco"s)
		{
			LoadModule(p.path().filename().string(), p.path().string());
		}
	}
#endif

	return &getInterface;
}

