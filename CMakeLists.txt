cmake_minimum_required(VERSION 3.10)
project(test_x11)
set(CMAKE_C_STANDARD 11)

add_executable(main src/main.c)
target_link_libraries(main X11)
target_link_libraries(main xcb)
target_link_libraries(main xcb-util)
target_link_libraries(main xcb-icccm)
