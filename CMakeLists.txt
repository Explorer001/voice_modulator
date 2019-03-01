cmake_minimum_required(VERSION 3.1)
project(voice_modulator)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -lasound -ljack -lpthread -lportaudio")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_executable(modulate
	      ${CMAKE_CURRENT_SOURCE_DIR}/src/modulate.c)

target_link_libraries(modulate portaudio)
target_link_libraries(modulate m)
