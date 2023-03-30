cmake_minimum_required(VERSION 3.22)
project(dante_clion C)

set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "-O3")



add_executable(dante_clion main.c bitmaprender.c bitmaprender.h)

