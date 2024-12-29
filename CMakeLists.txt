cmake_minimum_required(VERSION 3.31.1)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(AngelscriptProfiler)

find_package(SDL2 REQUIRED)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(OpenGL REQUIRED)
find_package(Angelscript REQUIRED)
find_package(spdlog REQUIRED)

add_executable(AngelscriptProfiler WIN32)

target_compile_features(AngelscriptProfiler PRIVATE cxx_std_23)

target_include_directories(AngelscriptProfiler
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/src/imgui)

if (TARGET SDL2::SDL2main)
	target_link_libraries(AngelscriptProfiler PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(AngelscriptProfiler
	PRIVATE
		SDL2::SDL2
		OpenGL::GL
		Angelscript::angelscript
		spdlog::spdlog)

target_sources(AngelscriptProfiler
	PRIVATE
		src/Application.cpp
		src/Application.hpp
		src/Main.cpp
		
		src/as/as_addons.cpp
		src/as/as_utils.hpp
		src/as/ScriptingSystem.cpp
		src/as/ScriptingSystem.hpp
		src/as/ScriptProfiler.cpp
		src/as/ScriptProfiler.hpp
		
		src/imgui/imconfig.h
		src/imgui/imgui.cpp
		src/imgui/imgui.h
		src/imgui/imgui.natstepfilter
		src/imgui/imgui.natvis
		src/imgui/imgui_demo.cpp
		src/imgui/imgui_draw.cpp
		src/imgui/imgui_internal.h
		src/imgui/imgui_stdlib.cpp
		src/imgui/imgui_stdlib.h
		src/imgui/imgui_tables.cpp
		src/imgui/imgui_widgets.cpp
		src/imgui/imstb_rectpack.h
		src/imgui/imstb_textedit.h
		src/imgui/imstb_truetype.h
		src/imgui/backends/imgui_impl_opengl3.cpp
		src/imgui/backends/imgui_impl_opengl3.h
		src/imgui/backends/imgui_impl_opengl3_loader.h
		src/imgui/backends/imgui_impl_sdl2.cpp
		src/imgui/backends/imgui_impl_sdl2.h
		
		src/implot/implot.cpp
		src/implot/implot.h
		src/implot/implot_demo.cpp
		src/implot/implot_internal.h
		src/implot/implot_items.cpp)

get_target_property(TARGET_SOURCES AngelscriptProfiler SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${TARGET_SOURCES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AngelscriptProfiler)
