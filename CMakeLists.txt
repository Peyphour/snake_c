cmake_minimum_required(VERSION 3.7)
project(untitled)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c graphics.c graphics.h list.h list.c)
add_executable(untitled ${SOURCE_FILES})
target_link_libraries(untitled m SDL SDL_net SDL_ttf pthread X11)