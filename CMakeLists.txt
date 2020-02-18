cmake_minimum_required(VERSION 3.10)
project(ai-race-sim)

include_directories(
  "./include"
  "/usr/local/include")

set(CMAKE_CXX_FLAGS "-pthread -std=c++11 -Wall -O2 -g")

add_subdirectory(cc)
