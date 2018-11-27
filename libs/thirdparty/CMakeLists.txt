#--------- Wyrd's third party sources
set( CRC32C_SRC
		crc32c/crc32c.cpp
		crc32c/crc32c.h)

set( CITYHASH_SRC
		cityhash/city.cc
		cityhash/city.h
		)

set( FMT_SRC
		fmt/src/format.cc
		fmt/src/posix.cc
		)

set( LZ4_SRC
		lz4/lz4.c
		lz4/lz4.h
		lz4/lz4frame.c
		lz4/lz4frame.h
		lz4/lz4frame_static.h
		lz4/lz4hc.c
		lz4/lz4hc.h
		lz4/xxhash.c
		lz4/xxhash.h
		)

set(IMGUI_SRC
		imgui/imconfig.h
		imgui/imgui.h
		imgui/imgui.cpp
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_widgets.cpp
		)

set(ASIO_SRC
		asio/src/asio.cpp
		asio/include/asio.hpp
		asio/include/asio/associated_allocator.hpp
		asio/include/asio/associated_executor.hpp
		asio/include/asio/async_result.hpp
		asio/include/asio/basic_datagram_socket.hpp
		asio/include/asio/basic_deadline_timer.hpp
		asio/include/asio/basic_io_object.hpp
		asio/include/asio/basic_raw_socket.hpp
		asio/include/asio/basic_seq_packet_socket.hpp
		asio/include/asio/basic_serial_port.hpp
		asio/include/asio/basic_signal_set.hpp
		asio/include/asio/basic_socket.hpp
		asio/include/asio/basic_socket_acceptor.hpp
		asio/include/asio/basic_socket_iostream.hpp
		asio/include/asio/basic_socket_streambuf.hpp
		asio/include/asio/basic_stream_socket.hpp
		asio/include/asio/basic_streambuf.hpp
		asio/include/asio/basic_streambuf_fwd.hpp
		asio/include/asio/basic_waitable_timer.hpp
		asio/include/asio/bind_executor.hpp
		asio/include/asio/buffer.hpp
		asio/include/asio/buffered_read_stream.hpp
		asio/include/asio/buffered_read_stream_fwd.hpp
		asio/include/asio/buffered_stream.hpp
		asio/include/asio/buffered_stream_fwd.hpp
		asio/include/asio/buffered_write_stream.hpp
		asio/include/asio/buffered_write_stream_fwd.hpp
		asio/include/asio/buffers_iterator.hpp
		asio/include/asio/completion_condition.hpp
		asio/include/asio/connect.hpp
		asio/include/asio/coroutine.hpp
		asio/include/asio/datagram_socket_service.hpp
		asio/include/asio/deadline_timer.hpp
		asio/include/asio/deadline_timer_service.hpp
		asio/include/asio/defer.hpp
		asio/include/asio/dispatch.hpp
		asio/include/asio/error.hpp
		asio/include/asio/error_code.hpp
		asio/include/asio/execution_context.hpp
		asio/include/asio/executor.hpp
		asio/include/asio/executor_work_guard.hpp
		asio/include/asio/experimental.hpp
		asio/include/asio/handler_alloc_hook.hpp
		asio/include/asio/handler_continuation_hook.hpp
		asio/include/asio/handler_invoke_hook.hpp
		asio/include/asio/handler_type.hpp
		asio/include/asio/high_resolution_timer.hpp
		asio/include/asio/io_context.hpp
		asio/include/asio/io_context_strand.hpp
		asio/include/asio/io_service.hpp
		asio/include/asio/io_service_strand.hpp
		asio/include/asio/is_executor.hpp
		asio/include/asio/is_read_buffered.hpp
		asio/include/asio/is_write_buffered.hpp
		asio/include/asio/packaged_task.hpp
		asio/include/asio/placeholders.hpp
		asio/include/asio/post.hpp
		asio/include/asio/raw_socket_service.hpp
		asio/include/asio/read.hpp
		asio/include/asio/read_at.hpp
		asio/include/asio/read_until.hpp
		asio/include/asio/seq_packet_socket_service.hpp
		asio/include/asio/serial_port.hpp
		asio/include/asio/serial_port_base.hpp
		asio/include/asio/serial_port_service.hpp
		asio/include/asio/signal_set.hpp
		asio/include/asio/signal_set_service.hpp
		asio/include/asio/socket_acceptor_service.hpp
		asio/include/asio/socket_base.hpp
		asio/include/asio/spawn.hpp
		asio/include/asio/ssl.hpp
		asio/include/asio/steady_timer.hpp
		asio/include/asio/strand.hpp
		asio/include/asio/stream_socket_service.hpp
		asio/include/asio/streambuf.hpp
		asio/include/asio/system_context.hpp
		asio/include/asio/system_error.hpp
		asio/include/asio/system_executor.hpp
		asio/include/asio/system_timer.hpp
		asio/include/asio/thread.hpp
		asio/include/asio/thread_pool.hpp
		asio/include/asio/time_traits.hpp
		asio/include/asio/unyield.hpp
		asio/include/asio/use_future.hpp
		asio/include/asio/uses_executor.hpp
		asio/include/asio/version.hpp
		asio/include/asio/wait_traits.hpp
		asio/include/asio/waitable_timer_service.hpp
		asio/include/asio/write.hpp
		asio/include/asio/write_at.hpp
		asio/include/asio/yield.hpp
		)
set(ICONFONTCPPHEADERS_SRC
		IconFontCppHeaders/IconsFontAwesome4.h
		IconFontCppHeaders/IconsFontAwesome5.h
		IconFontCppHeaders/IconsFontAwesome5Brands.h
		IconFontCppHeaders/IconsFontAwesome5Pro.h
		IconFontCppHeaders/IconsFontAwesome5ProBrands.h
		IconFontCppHeaders/IconsForkAwesome.h
		IconFontCppHeaders/IconsIonicons.h
		IconFontCppHeaders/IconsKenney.h
		IconFontCppHeaders/IconsMaterialDesign.h
		IconFontCppHeaders/IconsMaterialDesignIcons.h
		)
set( NLOHMANN_SRC nlohmann/json.h)
set(CLIPP_SRC clipp/clipp.h)

option(SHADERC_SKIP_INSTALL "" ON)
option(SHADERC_SKIP_TESTS "" ON)
option(SHADERC_ENABLE_NV_EXTENSIONS "" ON)
add_subdirectory(shaderc)


set( THIRDPARTY_LIBS enkiTS)
foreach(LIB ${THIRDPARTY_LIBS})
	add_subdirectory(${LIB})
endforeach(LIB)
add_subdirectory(tbb)

add_library( asio STATIC ${ASIO_SRC} )
target_include_directories(asio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/asio/include )
target_compile_definitions(asio PUBLIC
		-DASIO_STANDALONE=1
		-DASIO_SEPARATE_COMPILATION=1
		-DASIO_NO_DEPRECATED=1)
if (WIN32)
	target_compile_definitions(asio PUBLIC
		-D_WIN32_WINNT=0x603)
endif(WIN32)

add_definitions(${wyrd_DEFINITIONS})
include_directories(${wyrd_INCLUDES})

add_library( imgui ${IMGUI_SRC} )

set( THIRDPARTY_LIBS ${THIRDPARTY_LIBS} tbb_static tbbmalloc_static)

add_library( thirdparty STATIC
		${CRC32C_SRC}
		${CITYHASH_SRC}
		${FMT_SRC}
		${LZ4_SRC}
		${ICONFONTCPPHEADERS_SRC}
		${NLOHMANN_SRC}
		${CLIPP_SRC}
		)
target_link_libraries(thirdparty ${THIRDPARTY_LIBS})


