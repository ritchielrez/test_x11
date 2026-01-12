cmake_minimum_required(VERSION 3.10)
project(glad)
set(CMAKE_C_STANDARD 11)

add_library(glad STATIC src/gl.c src/egl.c src/glx.c)
target_include_directories(glad PUBLIC include/)

