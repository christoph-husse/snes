cmake_minimum_required(VERSION 2.8)

include_directories(
	${CMAKE_SOURCE_DIR}/librenderer/
  ${CMAKE_SOURCE_DIR}/libsnes/
  ${CMAKE_SOURCE_DIR}/snes-player/
)

add_definitions(-DHAVE_STDINT_H=1 -DHAVE_SYS_IOCTL_H=1)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/snes-player/*")

add_executable(snes-player WIN32 ${SOURCES})
target_link_libraries(snes-player renderer snes gameconsole)
