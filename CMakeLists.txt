
# Drunkard library.
cmake_minimum_required(VERSION 2.6)
project(drunkard)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}")

set(SOURCES
    src/drunkard.c
    src/drunkard_utils.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

add_library(
    drunkard
    ${SOURCES}
)

install_files(/include FILES include/drunkard.h include/drunkard_utils.h)
install_files(/lib FILES lib/libdrunkard.a)

# Examples

find_library(LIBTCOD_LIBRARIES libtcod.so
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64)

find_path(LIBTCOD_INCLUDE_DIRS libtcod.h
    /usr/include
    /usr/local/include
    /usr/include/libtcod
    /usr/local/include/libtcod)

if(LIBTCOD_INCLUDE_DIRS)
    include_directories(${LIBTCOD_INCLUDE_DIRS})
endif(LIBTCOD_INCLUDE_DIRS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples)

add_executable(cave examples/cave.c)
target_link_libraries(cave drunkard m)

add_executable(atw examples/atw.c)
target_link_libraries(atw drunkard m)

add_executable(cave2 examples/cave2.c)
target_link_libraries(cave2 drunkard m)

add_executable(dung examples/dung.c)
target_link_libraries(dung drunkard m)

add_executable(utils_example examples/utils_example.c)
target_link_libraries(utils_example drunkard m)

if(LIBTCOD_LIBRARIES)
    add_executable(screen_shotter examples/screen_shotter.c)
    target_link_libraries(screen_shotter drunkard m ${LIBTCOD_LIBRARIES})
endif(LIBTCOD_LIBRARIES)
