cmake_minimum_required(VERSION 3.9)
project(viz)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(bgfx.cmake)

include( CMakeParseArguments )
include( bgfx.cmake/cmake/util/ConfigureDebugging.cmake )
include( bgfx.cmake/cmake/3rdparty/ib-compress.cmake )
include( bgfx.cmake/cmake/3rdparty/dear-imgui.cmake )

function( add_bgfx_shader FILE FOLDER)
	get_filename_component( FILENAME "${FILE}" NAME_WE )
	string( SUBSTRING "${FILENAME}" 0 2 TYPE )
	if( "${TYPE}" STREQUAL "fs" )
		set( TYPE "FRAGMENT" )
		set( D3D_PREFIX "ps" )
	elseif( "${TYPE}" STREQUAL "vs" )
		set( TYPE "VERTEX" )
		set( D3D_PREFIX "vs" )
	else()
		set( TYPE "" )
	endif()
	if( NOT "${TYPE}" STREQUAL "" )
		set( COMMON FILE ${FILE} ${TYPE} INCLUDES ${BGFX_DIR}/src )
		set( OUTPUTS "" )
		set( OUTPUTS_PRETTY "" )
		if( WIN32 )
			# dx9
			set( DX9_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/dx9/${FILENAME}.bin )
			shaderc_parse( DX9 ${COMMON} WINDOWS PROFILE ${D3D_PREFIX}_3_0 OUTPUT ${DX9_OUTPUT} )
			list( APPEND OUTPUTS "DX9" )
			set( OUTPUTS_PRETTY "${OUTPUTS_PRETTY}DX9, " )

			# dx11
			set( DX11_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/dx11/${FILENAME}.bin )
			shaderc_parse( DX11 ${COMMON} WINDOWS PROFILE ${D3D_PREFIX}_4_0 OUTPUT ${DX11_OUTPUT} )
			list( APPEND OUTPUTS "DX11" )
			set( OUTPUTS_PRETTY "${OUTPUTS_PRETTY}DX11, " )
		endif()
		if( APPLE )
			# metal
			set( METAL_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/metal/${FILENAME}.bin )
			shaderc_parse( METAL ${COMMON} OSX OUTPUT ${METAL_OUTPUT} )
			list( APPEND OUTPUTS "METAL" )
			set( OUTPUTS_PRETTY "${OUTPUTS_PRETTY}Metal, " )
		endif()
		# gles
		set( GLES_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gles/${FILENAME}.bin )
		shaderc_parse( GLES ${COMMON} ANDROID OUTPUT ${GLES_OUTPUT} )
		list( APPEND OUTPUTS "GLES" )
		set( OUTPUTS_PRETTY "${OUTPUTS_PRETTY}GLES, " )
		# glsl
		set( GLSL_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/glsl/${FILENAME}.bin )
		shaderc_parse( GLSL ${COMMON} LINUX PROFILE 120 OUTPUT ${GLSL_OUTPUT} )
		list( APPEND OUTPUTS "GLSL" )
		set( OUTPUTS_PRETTY "${OUTPUTS_PRETTY}GLSL" )
		set( OUTPUT_FILES "" )
		set( COMMANDS "" )
		foreach( OUT ${OUTPUTS} )
			list( APPEND OUTPUT_FILES ${${OUT}_OUTPUT} )
			list( APPEND COMMANDS COMMAND "$<TARGET_FILE:shaderc>" ${${OUT}} )
			get_filename_component( OUT_DIR ${${OUT}_OUTPUT} DIRECTORY )
			file( MAKE_DIRECTORY ${OUT_DIR} )
		endforeach()
		file( RELATIVE_PATH PRINT_NAME ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${FILE} )
		add_custom_command(
			MAIN_DEPENDENCY
			${FILE}
			OUTPUT
			${OUTPUT_FILES}
			${COMMANDS}
			COMMENT "Compiling shader ${PRINT_NAME} for ${OUTPUTS_PRETTY}"
		)
	endif()
endfunction()

set( SOURCES "" )
set( SHADERS "" )

set(DIRECTORIES
	${BGFX_DIR}/examples/common/debugdraw
	${BGFX_DIR}/examples/common/entry
	${BGFX_DIR}/examples/common/font
	${BGFX_DIR}/examples/common/imgui
	${BGFX_DIR}/examples/common/nanovg
	${BGFX_DIR}/examples/common/ps
	${CMAKE_CURRENT_SOURCE_DIR}/shaders_src
)

foreach( DIR ${DIRECTORIES} )
	if( APPLE )
		file( GLOB GLOB_SOURCES ${DIR}/*.mm )
		list( APPEND SOURCES ${GLOB_SOURCES} )
	endif()
	file( GLOB GLOB_SOURCES ${DIR}/*.c ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.sc )
	list( APPEND SOURCES ${GLOB_SOURCES} )
	file( GLOB GLOB_SHADERS ${DIR}/*.sc )
	list( APPEND SHADERS ${GLOB_SHADERS} )
endforeach()

add_library( bgfx-ex-common STATIC EXCLUDE_FROM_ALL ${SOURCES} )
target_link_libraries( bgfx-ex-common PUBLIC bgfx ib-compress dear-imgui )
if( UNIX AND NOT APPLE )
	target_link_libraries( bgfx-ex-common PUBLIC X11 )
endif()

set(VIZ_SOURCE "")
list( APPEND VIZ_SOURCES 
    main.cpp
    Imgui-IGS-Snippet/ImguiWindowsFileIO.cpp
    Imgui-IGS-Snippet/ImguiWindowsFileIO.hpp
    json.h
    guizmo_math.h
    propertyeditor.h
    optional_wrapper.h
	vectordisplay.cpp
	vectordisplay.h
	)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${BGFX_DIR}/examples/common)
add_executable( viz WIN32 ${VIZ_SOURCES} ${SHADERS} )
add_definitions( -DUSING_STATIC_LIBS )
target_link_libraries( viz example-common wyrd_static binify)
if( MSVC )
	set_target_properties( viz PROPERTIES 
			LINK_FLAGS "/ENTRY:\"mainCRTStartup\"" 
			VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_PROJECT_DIR}"
			)
endif()
target_compile_definitions( viz PRIVATE 
		"-D_CRT_SECURE_NO_WARNINGS" "-D__STDC_FORMAT_MACROS" "-DENTRY_CONFIG_IMPLEMENT_MAIN=1" )

# Configure shaders
foreach( SHADER ${SHADERS} )
	add_bgfx_shader( ${SHADER} .)
endforeach()

source_group( "Shader Files" FILES ${SHADERS})
