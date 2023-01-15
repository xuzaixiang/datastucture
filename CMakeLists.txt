cmake_minimum_required(VERSION 3.17)
project(datastructure C)

set(CMAKE_C_STANDARD 99)
add_library(ds ds_rhmap.c)

include_directories(.)
add_subdirectory(test)

