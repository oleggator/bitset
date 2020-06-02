cmake_minimum_required(VERSION 2.8)
project(bitset C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-undefined dynamic_lookup")


include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_library(bitset SHARED bitset.c bitset.h)
