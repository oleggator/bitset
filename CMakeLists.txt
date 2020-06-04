cmake_minimum_required(VERSION 2.8)
project(bitset C)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O3 -mavx -ftree-vectorize")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Init msgpuck submodule
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif ()
    endif ()
endif ()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/msgpuck/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif ()
add_subdirectory(msgpuck)

add_library(bitset_internal SHARED bitset.c bitset.h)
target_link_libraries(bitset_internal msgpuck)

set_target_properties(bitset_internal PROPERTIES
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME "bitset")

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
install(TARGETS bitset_internal LIBRARY DESTINATION ${TARANTOOL_INSTALL_LIBDIR})
