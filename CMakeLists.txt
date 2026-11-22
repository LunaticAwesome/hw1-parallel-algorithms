cmake_minimum_required(VERSION 3.29)
project(Task1)

include(FetchContent)
FetchContent_Declare(parlaylib
        GIT_REPOSITORY  https://github.com/cmuparlay/parlaylib.git
        GIT_TAG         master
)
FetchContent_GetProperties(parlaylib)
if(NOT parlaylib_POPULATED)
    FetchContent_Populate(parlaylib)
    add_subdirectory(${parlaylib_SOURCE_DIR} EXCLUDE_FROM_ALL)
endif()

set(CMAKE_CXX_STANDARD 20)

add_executable(Task1 main.cpp
        quick_sort.h)

target_link_libraries(Task1 PUBLIC parlay)
