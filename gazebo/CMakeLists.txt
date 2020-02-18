CMAKE_MINIMUM_REQUIRED(VERSION 3.5 FATAL_ERROR)
project(gz_rccar)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -O2")
add_subdirectory(plugins)
