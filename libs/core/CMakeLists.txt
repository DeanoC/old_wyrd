set( CORE_SRC
		any.h
		core.h
		exception.h
		filesystem.h
		freelist.h
		linear_allocator.h
		loguru.hpp
		platform.h
		platform_cuda.h
		platform_linux.h
		platform_osx.h
		platform_posix.h
		platform_win.h
		quick_hash.h
		utils.h
		)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

# currently core is a header only library
#add_library( core STATIC ${CORE_SRC} )
add_library( core INTERFACE )
