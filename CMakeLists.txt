set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_STANDARD 20)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")

cmake_minimum_required(VERSION 3.2)

project(console-engine)

include_directories(
    include
)

add_library(console src/console.cc)
