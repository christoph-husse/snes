cmake_minimum_required(VERSION 2.8)

project (snes-player)

IF(DEFINED EMSCRIPTEN)

 set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ALL_FLAGS} --js-library \"${CMAKE_SOURCE_DIR}/snes-player/native_impl.js\" -s EXPORTED_FUNCTIONS=\"['_input_OnKeyboardEvent', '_input_OnGestureEvent', '_pcmPlayback_GetBufferAt', '_pcmPlayback_PrepareBuffer', '_main']\" --embed-file ALinkToThePast.smc -s TOTAL_MEMORY=134217728 -O3 -s SAFE_HEAP=1")

 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALL_FLAGS} " )
ENDIF()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -w -O3 -fno-omit-frame-pointer -g" )
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")
set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g -lpthread")

add_subdirectory(libsnes)
add_subdirectory(libgameconsole)
add_subdirectory(librenderer)
add_subdirectory(snes-player)
