set( RENDER_SRC
		generictextureformat.h
		gtfcracker.h
		stable.h
		device.h
		display.h
		texture.cpp
		texture.h
		image.inl
		image_fetch.inl
		image_put.inl
		image.h
		resources.cpp
		resources.h
		commandqueue.h
		encoder.h
		renderpass.h
		rendertarget.h
		semaphore.h
		fence.h
		types.h
		renderpass.cpp
		image.cpp
		rendertarget.cpp
		shader.cpp
		shader.h
		pipeline.cpp
		pipeline.h
		bindingtable.cpp
		bindingtable.h
		ropblender.cpp
		ropblender.h
		viewport.cpp
		viewport.h
		rasterisationstate.cpp
		rasterisationstate.h
		vertexinput.cpp
		vertexinput.h
		buffer.cpp
		buffer.h)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES})

add_library( render STATIC ${RENDER_SRC} )
