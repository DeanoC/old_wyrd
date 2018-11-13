set( VULKAN_SRC
		vkfcracker.h
		system.h
		system.cpp
		functionlist.inl
		api.cpp
		api.h
		display.h
		device.cpp
		device.h
		vk_mem_alloc.h
		vk_mem_alloc.cpp
		texture.cpp
		texture.h
		display.cpp
		commandqueue.cpp
		commandqueue.h
		encoder.cpp
		encoder.h
		rendertarget.cpp
		rendertarget.h
		fence.cpp
		fence.h
		renderpass.cpp
		renderpass.h
		encoderpool.cpp
		encoderpool.h
		renderencoder.cpp
		renderencoder.h
		computeencoder.cpp
		computeencoder.h
		types.h
		resources.cpp
		resources.h
		semaphore.cpp
		semaphore.h
		pipeline.cpp
		pipeline.h
		shadermodule.cpp
		shadermodule.h
		bindingtable.cpp
		bindingtable.h
		buffer.cpp
		buffer.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( vulkan STATIC ${VULKAN_SRC} )
