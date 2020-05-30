cmake_minimum_required(VERSION 2.8)
project(bitset C)

set(CMAKE_C_STANDARD 99)

include_directories(/usr/local/include /usr/local/include/tarantool)

add_library(bitset SHARED bitset.c bitset.h)
