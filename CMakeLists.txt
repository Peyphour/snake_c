cmake_minimum_required(VERSION 3.7)
project(snake)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c types.h constants.h)
add_executable(snake ${SOURCE_FILES})
target_link_libraries(snake m SDL SDL_net SDL_ttf pthread X11 linked_list graphics)