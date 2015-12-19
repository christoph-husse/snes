cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_SOURCE_DIR}/librenderer/)

IF(NOT DEFINED EMSCRIPTEN)
	include(FindPkgConfig)
	pkg_search_module(SDL2 REQUIRED sdl2)
	include_directories(${SDL2_INCLUDE_DIRS})
ENDIF()
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/librenderer/*")

add_library(renderer STATIC ${SOURCES})

IF(NOT DEFINED EMSCRIPTEN)
	target_link_libraries(renderer ${SDL2_LIBRARIES})
ENDIF()
