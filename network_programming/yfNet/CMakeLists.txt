cmake_minimum_required(VERSION 3.17)
project(yfNet)

set(CMAKE_CXX_STANDARD 14)

add_executable(yfNet main.cpp NetworkSocket.h NetOps.h NetOps.cpp)