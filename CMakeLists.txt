cmake_minimum_required(VERSION 3.21)
project(Test1)

set(CMAKE_CXX_STANDARD 20)

add_executable(Test1 main.cpp Pop.cpp Pop.h Factory.cpp Factory.h)
