cmake_minimum_required(VERSION 3.9)

include(cmake/ProjectOptions.cmake)

project(
    console
    VERSION 0.0.1
    DESCRIPTION "Library for interacting with the console."
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

set_project_options("include" "")

add_subdirectory(src)
