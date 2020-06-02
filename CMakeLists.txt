cmake_minimum_required(VERSION 2.8)
project(bitset C)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "-undefined dynamic_lookup")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_library(bitset_internal SHARED bitset.c bitset.h)

set_target_properties(bitset_internal PROPERTIES
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME "internal")

set(LUA_PATH "LUA_PATH=${PROJECT_SOURCE_DIR}/?.lua\\;${PROJECT_SOURCE_DIR}/?/init.lua\\;\\;")
set(LUA_SOURCE_DIR "LUA_SOURCE_DIR=${PROJECT_SOURCE_DIR}")

execute_process(
        COMMAND git describe --tags --always
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE GIT_DESCRIBE
        ERROR_QUIET
)

if (NOT GIT_DESCRIBE)
    set(GIT_DESCRIBE "unknown")
endif ()

configure_file(
        "${PROJECT_SOURCE_DIR}/VERSION.lua.in"
        "${CMAKE_CURRENT_BINARY_DIR}/VERSION.lua"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION.lua
        DESTINATION ${TARANTOOL_INSTALL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS bitset_internal LIBRARY DESTINATION ${TARANTOOL_INSTALL_LIBDIR}/bitset)
install(FILES bitset.lua DESTINATION ${TARANTOOL_INSTALL_LUADIR}/bitset)
