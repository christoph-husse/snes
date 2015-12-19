cmake_minimum_required(VERSION 2.8)

include_directories(
	${CMAKE_SOURCE_DIR}/libsnes/
)

add_definitions(-DHAVE_STDINT_H=1 -DHAVE_SYS_IOCTL_H=1)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/libsnes/*")

add_library(snes STATIC ${SOURCES})
