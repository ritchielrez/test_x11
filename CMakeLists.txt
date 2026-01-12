cmake_minimum_required(VERSION 3.10)
project(test_x11)
set(CMAKE_C_STANDARD 11)

add_executable(main src/main.c)

target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_subdirectory(vendor/glad)
target_link_libraries(main glad)

target_link_libraries(main X11)
target_link_libraries(main X11-xcb)
target_link_libraries(main xcb)
target_link_libraries(main xcb-util)
target_link_libraries(main xcb-icccm)
target_link_libraries(main GL)
target_link_libraries(main EGL)
