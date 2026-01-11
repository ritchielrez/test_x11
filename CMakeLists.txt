cmake_minimum_required(VERSION 3.10)
project(test_x11)
set(CMAKE_C_STANDARD 11)

add_executable(main src/main.c)

if (MSVC)
    target_compile_options(main PRIVATE /W4 /WX)
else ()
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

add_subdirectory(vendor/glad)
target_link_libraries(main glad)

target_link_libraries(main X11)
target_link_libraries(main X11-xcb)
target_link_libraries(main xcb)
target_link_libraries(main xcb-util)
target_link_libraries(main xcb-icccm)
